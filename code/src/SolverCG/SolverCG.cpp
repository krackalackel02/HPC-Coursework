/**
 * @file SolverCG.cpp
 * @author Paul Kallarackel
 * @brief This file details the methods used to solve the system of equations in space for the lid-drigven cavity att a given timestep
 * @version 0.1
 * @date 2024-02-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <iostream>
#include <algorithm>
#include <cstring>
using namespace std;
#include <cmath>
#include <unistd.h>

#include <cblas.h>

#include "../../include/SolverCG.h"
#include "../../include/ParallelFunc.h"
/**
 * @brief Used as shorthand for coordinates
 * 
 */
#define IDX(I,J) ((J)*Nx + (I))

/**
 * @brief Construct a new Solver CG:: Solver CG object. This  instantiates vectors to be used in conjugate gradient algorithm
 *
    * @param pNx /// Number of x points
    * @param pNy /// Number of y points
    * @param pdx /// X step size
    * @param pdy /// Y step size
 */
SolverCG::SolverCG(int pNx, int pNy, double pdx, double pdy,prl::gridData* GRID)
{
    /// Initialise domain discretisation and state vectors
    dx = pdx;
    dy = pdy;
    Nx = pNx;
    Ny = pNy;
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    int size = (Chunky + 2) * (Chunkx + 2);
    r = new double[size];
    p = new double[size];
    z = new double[size];
    t = new double[size]; //temp
    
}

/**
 * @brief Destroy the Solver CG:: Solver CG object
 * 
 */
SolverCG::~SolverCG()
{
    /// free up memory
    if (r)
    {
        delete[] r;
        delete[] p;
        delete[] z;
        delete[] t;
    }
}

/**
 * @brief Implements conjugate gradient algorithm using cblas
 * 
 * @param b /// Vector Result B
 * @param x /// Vector State X0, initial guess for solution on each time step
 */
void SolverCG::Solve(double* b, double* x,prl::gridData* GRID) {
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    unsigned int n =(Chunkx+2)*(Chunky+2);
    int k;
    double alpha=0.0;
    double alphanum=0.0;
    double alphaden=0.0;

    double localphanum=0.0;
    double localphaden=0.0;
    double beta=0.0;
    double betanum=0.0;
    double betaden=0.0;

    double locbetanum=0.0;
    double locbetaden=0.0;
    double eps; /// current error
    double loceps = 0.0;; /// current error
    double tol = 0.001; /// tolerance of solver

    /// Print out current error, calculated using norm-2
    loceps=0.0;
    GRID->edgeZero(b);

    // Instead of  cblas_dnrm2(n, b, 1);
    #pragma omp parallel for schedule(static)reduction(+:loceps)
    for (unsigned int i = 0; i < n; ++i) {
        loceps += b[i] * b[i];
    }
    loceps = sqrt(loceps);
    MPI_Allreduce(&loceps,&eps,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

    if (eps < tol*tol) {
        // Instead of   std::fill(x, x+n, 0.0);
        #pragma omp parallel for schedule(static)
        for (unsigned int i = 0; i < n; ++i) {
            x[i] = 0.0;
        }
        if(GRID->getCenter()==0)cout << "Norm is " << eps << endl;
        return;
    }

    /// Initialise conjugate gradient algorithm with correct states and boundary conditions.
    ApplyOperator(x, t,GRID);

    // Instead of cblas_dcopy
    #pragma omp parallel for schedule(static)
    for (unsigned int i = 0; i < n; ++i) {
        r[i] = b[i];
    }
    // cblas_dcopy(n, b, 1, r, 1);        // r_0 = b (i.e. b)

    ImposeBC(r,GRID);
    
    // Instead of cblas_daxpy
    #pragma omp parallel for schedule(static)
    for (unsigned int i = 0; i < n; ++i) {
        r[i] = r[i]-t[i];
    }
    // cblas_daxpy(n, -1.0, t, 1, r, 1);

    
    /// solve for outer walls
    Precondition(r, z,GRID);
    
    // Instead of cblas_dcopy
    #pragma omp parallel for schedule(static)
    for (unsigned int i = 0; i < n; ++i) {
        p[i] = z[i];
    }
    // cblas_dcopy(n, z, 1, p, 1);        // p_0 = r_0


    k = 0;
    /// Using conjugate gradient method to solve for new vorticity
    do {
        k++;
        // Perform action of Nabla^2 * p

        ApplyOperator(p, t,GRID);

        // Instead of localphaden = cblas_ddot(n, t, 1, p, 1);
        localphaden = 0.0;
        #pragma omp parallel for schedule(static)reduction(+:localphaden)
        for (unsigned int i = 0; i < n; ++i) {
            localphaden += p[i] * t[i];
        }
        // localphaden = cblas_ddot(n, t, 1, p, 1);  // alpha = p_k^T A p_k
        MPI_Allreduce(&localphaden,&alphaden,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

        localphanum = 0.0;
        // Instead of localphanum = cblas_ddot(n, r, 1, z, 1);
        #pragma omp parallel for schedule(static)reduction(+:localphanum)
        for (unsigned int i = 0; i < n; ++i) {
            localphanum += r[i] * z[i];
        }
        // localphanum = cblas_ddot(n, r, 1, z, 1); // compute alpha_k
        MPI_Allreduce(&localphanum,&alphanum,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

        alpha = alphanum/alphaden;
        // if(GRID->getCenter()==0&&k==1)cout << "MPI alpha: "<<alpha << endl;

        // Instead of locbetaden  = cblas_ddot(n, r, 1, z, 1)
        locbetaden = 0.0;
        #pragma omp parallel for schedule(static)reduction(+:locbetaden)
        for (unsigned int i = 0; i < n; ++i) {
            locbetaden += r[i] * z[i];
        }
        // locbetaden  = cblas_ddot(n, r, 1, z, 1);  // z_k^T r_k
        MPI_Allreduce(&locbetaden,&betaden,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

        // Instead of cblas_daxpy
        #pragma omp parallel for schedule(static)
        for (unsigned int i = 0; i < n; ++i) {
            x[i] = x[i]+alpha*p[i]; 
        }
        // cblas_daxpy(n,  alpha, p, 1, x, 1);  // x_{k+1} = x_k + alpha_k p_k

        // Instead of cblas_daxpy
        #pragma omp parallel for schedule(static)
        for (unsigned int i = 0; i < n; ++i) {
            r[i] = r[i]-alpha*t[i];
        }
        // cblas_daxpy(n, -alpha, t, 1, r, 1); // r_{k+1} = r_k - alpha_k A p_k

        loceps=0.0;
        GRID->edgeZero(r);
        // Instead of  cblas_dnrm2(n, r, 1);
        #pragma omp parallel for schedule(static)reduction(+:loceps)
        for (unsigned int i = 0; i < n; ++i) {
            loceps += r[i] * r[i];
        }
        loceps = sqrt(loceps);
        MPI_Allreduce(&loceps,&eps,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

        if (eps < tol*tol) {
            break;
        }
        Precondition(r, z,GRID);

        // Instead of locbetanum = cblas_ddot(n, r, 1, z, 1);
        locbetanum = 0.0;
        #pragma omp parallel for schedule(static)reduction(+:locbetanum)
        for (unsigned int i = 0; i < n; ++i) {
            locbetanum += r[i] * z[i];
        }
        // locbetanum = cblas_ddot(n, r, 1, z, 1);
        MPI_Allreduce(&locbetanum,&betanum,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
        beta = betanum/betaden;

        // Instead of cblas_dcopy
        #pragma omp parallel for schedule(static)
        for (unsigned int i = 0; i < n; ++i) {
            t[i] = z[i];
        }
        // cblas_dcopy(n, z, 1, t, 1);

        // Instead of cblas_daxpy
        #pragma omp parallel for schedule(static)
        for (unsigned int i = 0; i < n; ++i) {
            t[i] = t[i]+beta*p[i];
        }
        // cblas_daxpy(n, beta, p, 1, t, 1);

        // Instead of cblas_dcopy
        #pragma omp parallel for schedule(static)
        for (unsigned int i = 0; i < n; ++i) {
            p[i] = t[i];
        }
        // cblas_dcopy(n, t, 1, p, 1);

    } while (k < 5000); // Set a maximum number of iterations

    if (k == 5000) {
        if(GRID->getCenter()==0)cout << "FAILED TO CONVERGE" << endl;
        exit(-1);
    }

    if(GRID->getCenter()==0)cout << "Converged in " << k << " iterations. eps = " << eps << endl;
}

/**
 * @brief Implements the operation such that: out = A * in, where A is the -Nabla^2 where A*X = B
 * 
 * @param in Vector representing the input X
 * @param out Vector representing the output B: B = A*X
 */
void SolverCG::ApplyOperator(double* in, double* out,prl::gridData* GRID) {
    // Assume ordered with y-direction fastest (column-by-column)
    double dx2i = 1.0/dx/dx;
    double dy2i = 1.0/dy/dy;
    int* start = new int[2];
    int* stop = new int[2];
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    GRID->getStart(start);
    GRID->getStop(stop);
    GRID->exchangeGhost(in);
    bool wallTop = start[1]==0;
    bool wallBottom = stop[1]==Ny-1;
    bool wallLeft = start[0]==0;
    bool wallRight = stop[0]==Nx-1;

    #pragma omp parallel for schedule(static) collapse(2)
    for (int j = (!wallTop ? 0 : 1); j < (!wallBottom ? Chunky : Chunky - 1); ++j)
    {
        for (int i = (!wallLeft ? 0 : 1); i < (!wallRight ? Chunkx : Chunkx - 1); ++i)
        {

            out[LOCIDX(i, j)] = (-  in[LOCIDX(i - 1, j)]
                                 +  2.0 * in[LOCIDX(i, j)]
                                 -  in[LOCIDX(i + 1, j)]) * dx2i 
                            + (  -  in[LOCIDX(i, j-1)]
                                 +  2.0 * in[LOCIDX(i, j)]
                                 -  in[LOCIDX(i, j+1)]) * dy2i;
        }
    }
    GRID->edgeZero(out);
    GRID->edgeZero(in);
    delete[] start;
    delete[] stop;
}

/**
 * @brief Uses Jacobi Preconsitioning to scale based of eigenvalues of matrix
 * 
 * @param in residual error vector
 * @param out preconsitioned residual error vector
 */
void SolverCG::Precondition(double* in, double* out,prl::gridData* GRID) {
    int* start = new int[2];
    int *stop = new int[2];
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    GRID->getStart(start);
    GRID->getStop(stop);
    bool wallTop = start[1] == 0;
    bool wallBottom = stop[1] == Ny - 1;
    bool wallLeft = start[0] == 0;
    bool wallRight = stop[0] == Nx - 1;
    double dx2i = 1.0/dx/dx;
    double dy2i = 1.0/dy/dy;
    double factor = 2.0*(dx2i + dy2i);
    #pragma omp parallel for schedule(static) collapse(2)
    for (int j = (!wallTop ? 0 : 1); j < (!wallBottom ? Chunky : Chunky - 1); ++j)
    {
        for (int i = (!wallLeft ? 0 : 1); i < (!wallRight ? Chunkx : Chunkx - 1); ++i)
        {

            out[LOCIDX(i, j)] = in[LOCIDX(i, j)] /factor;
        }
    }
    if (wallTop)
    {
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < Chunkx; ++i)
        {
            out[LOCIDX(i, 0)] = in[LOCIDX(i, 0)];
        }
    }
    if (wallBottom)
    {
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < Chunkx; ++i)
        {
            out[LOCIDX(i, Chunky - 1)] = in[LOCIDX(i, Chunky - 1)];
        }
    }
    if (wallLeft)
    {
        #pragma omp parallel for schedule(static)
        for (int j = 0; j < Chunky; ++j)
        {
            out[LOCIDX(0, j)] = in[LOCIDX(0,j)];
        }
    }
    if (wallRight)
    {
        #pragma omp parallel for schedule(static)
        for (int j = 0; j < Chunky; ++j)
        {
            out[LOCIDX(Chunkx - 1, j)] = in[LOCIDX(Chunkx - 1, j)];
        }
    }
    delete[] start;
    delete[] stop;
}
/**
 * @brief Enforces zero initial vorticity boundary condition along each wall
 * 
 * @param inout Vorticity state vector
 */
void SolverCG::ImposeBC(double* inout,prl::gridData* GRID) {
    int* start = new int[2];
    int *stop = new int[2];
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    GRID->getStart(start);
    GRID->getStop(stop);
    bool wallTop = start[1] == 0;
    bool wallBottom = stop[1] == Ny - 1;
    bool wallLeft = start[0] == 0;
    bool wallRight = stop[0] == Nx - 1;
    // Boundaries
    if (wallTop)
    {
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < Chunkx; ++i)
        {
            inout[LOCIDX(i, 0)] = 0.0;
        }
    }
    if (wallBottom)
    {
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < Chunkx; ++i)
        {
            inout[LOCIDX(i, Chunky - 1)] = 0.0;
        }
    }
    if (wallLeft)
    {
        #pragma omp parallel for schedule(static)
        for (int j = 0; j < Chunky; ++j)
        {
            inout[LOCIDX(0, j)] = 0.0;
        }
    }
    if (wallRight)
    {
        #pragma omp parallel for schedule(static)
        for (int j = 0; j < Chunky; ++j)
        {
            inout[LOCIDX(Chunkx - 1, j)] = 0.0;
        }
    }
    delete[] start;
    delete[] stop;
}
