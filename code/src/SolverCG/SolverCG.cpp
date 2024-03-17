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
SolverCG::SolverCG(int pNx, int pNy, double pdx, double pdy)
{
    /// Initialise domain discretisation and state vectors
    dx = pdx;
    dy = pdy;
    Nx = pNx;
    Ny = pNy;
    int n = Nx*Ny;
    r = new double[n];
    p = new double[n];
    z = new double[n];
    t = new double[n]; //temp
    
}
SolverCG::SolverCG(int pNx, int pNy, double pdx, double pdy,prl::gridData* GRID)
{
    /// Initialise domain discretisation and state vectors
    dx = pdx;
    dy = pdy;
    Nx = pNx;
    Ny = pNy;
    int n = Nx*Ny;
    r = new double[n];
    p = new double[n];
    z = new double[n];
    t = new double[n]; //temp
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    int size = (Chunky + 2) * (Chunkx + 2);
    MPIr = new double[size];
    MPIp = new double[size];
    MPIz = new double[size];
    MPIt = new double[size]; //temp
    
}

/**
 * @brief Destroy the Solver CG:: Solver CG object
 * 
 */
SolverCG::~SolverCG()
{
    /// free up memory
    delete[] r;
    delete[] p;
    delete[] z;
    delete[] t;
    if (MPIr)
    {
        delete[] MPIr;
        delete[] MPIp;
        delete[] MPIz;
        delete[] MPIt;
    }
}

/**
 * @brief Implements conjugate gradient algorithm using cblas
 * 
 * @param b /// Vector Result B
 * @param x /// Vector State X0, initial guess for solution on each time step
 */
void SolverCG::Solve(double* b, double* x) {
    unsigned int n = Nx*Ny;
    int k;
    double alpha;
    double beta;
    double eps; /// current error
    double tol = 0.001; /// tolerance of solver

    /// Print out current error, calculated using norm-2
    eps = cblas_dnrm2(n, b, 1);
    if (eps < tol*tol) {
        std::fill(x, x+n, 0.0);
        cout << "Norm is " << eps << endl;
        return;
    }

    // Initialise conjugate gradient algorithm with correct states and boundary conditions.
    // if(Nx<20){

    //         cout << "SERIAL X:" << endl;

    // prl::PrintRowMatrix(Nx,Ny,x);
    // }
    // if(Nx<20){

    //         cout << "SERIAL B:" << endl;

    // prl::PrintRowMatrix(Nx,Ny,b);
    // }
    ApplyOperator(x, t);
    // if(Nx<20){

    //         cout << "SERIAL T:" << endl;

    // prl::PrintRowMatrix(Nx,Ny,t);
    // }
    cblas_dcopy(n, b, 1, r, 1);        // r_0 = b (i.e. b)
    ImposeBC(r);
    // if(Nx<20){

    //         cout << "SERIAL r:" << endl;

    // prl::PrintRowMatrix(Nx,Ny,r);
    // }

    cblas_daxpy(n, -1.0, t, 1, r, 1);
    // if(Nx<20){

    //         cout << "SERIAL Z:" << endl;

    // prl::PrintRowMatrix(Nx,Ny,z);
    // }
    /// solve for outer walls
    Precondition(r, z);
    cblas_dcopy(n, z, 1, p, 1);        // p_0 = r_0
    // if(Nx<20){

    //         cout << "SERIAL p:" << endl;

    // prl::PrintRowMatrix(Nx,Ny,p);
    // }

    k = 0;
    /// Using conjugate gradient method to solve for new vorticity
    do {
        k++;
        // Perform action of Nabla^2 * p
        ApplyOperator(p, t);
        //         if(k==1)cout << "SERIAL P" << endl;

        // if(k==1)prl::PrintRowMatrix(Nx,Ny,p);
        //         if(k==1)cout << "SERIAL Z:" << endl;

        // if(k==1)prl::PrintRowMatrix(Nx,Ny,z);
        //         if(k==1)cout << "SERIAL R:" << endl;

        // if(k==1)prl::PrintRowMatrix(Nx,Ny,r);
        // }
        alpha = cblas_ddot(n, t, 1, p, 1);  // alpha = p_k^T A p_k
        alpha = cblas_ddot(n, r, 1, z, 1) / alpha; // compute alpha_k
        beta  = cblas_ddot(n, r, 1, z, 1);  // z_k^T r_k
        // if(k==1)cout << "SERIAL alpha: "<<alpha << endl;

        cblas_daxpy(n,  alpha, p, 1, x, 1);  // x_{k+1} = x_k + alpha_k p_k
        cblas_daxpy(n, -alpha, t, 1, r, 1); // r_{k+1} = r_k - alpha_k A p_k

        eps = cblas_dnrm2(n, r, 1);


        if (eps < tol*tol) {
            // cout << "SERIAL ITerations: "<<k << endl;
            // cout << "SERIAL alpha: "<<alpha << endl;
            break;
        }
        Precondition(r, z);
        beta = cblas_ddot(n, r, 1, z, 1) / beta;
        // if(k==1)cout << "SERIAL beta: "<<beta << endl;

        cblas_dcopy(n, z, 1, t, 1);
        cblas_daxpy(n, beta, p, 1, t, 1);
        cblas_dcopy(n, t, 1, p, 1);

    } while (k < 5000); // Set a maximum number of iterations

    if (k == 5000) {
        cout << "FAILED TO CONVERGE" << endl;
        exit(-1);
    }

    // cout << "Converged in " << k << " iterations. eps = " << eps << endl;
}
void SolverCG::MPISolve(double* b, double* x,prl::gridData* GRID) {
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
    loceps = cblas_dnrm2(n, b, 1);
    MPI_Allreduce(&loceps,&eps,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
    eps = sqrt(eps);
    if (eps < tol*tol) {
        std::fill(x, x+n, 0.0);
        if(GRID->getCenter()==0)cout << "Norm is " << eps << endl;
        return;
    }

    /// Initialise conjugate gradient algorithm with correct states and boundary conditions.
    // MPI_Barrier(MPI_COMM_WORLD);
    // prl::time(GRID->getCenter()*1.5);
    // cout << "X RANK : "<<GRID->getCenter() << endl;
    // prl::PrintRowMatrix(6,7,x);

    // MPI_Barrier(MPI_COMM_WORLD);
    // prl::time(GRID->getCenter()*1.5);
    // cout << "B RANK : "<<GRID->getCenter() << endl;
    // prl::PrintRowMatrix(6,7,b);

    MPIApplyOperator(x, MPIt,GRID);
    
    // MPI_Barrier(MPI_COMM_WORLD);
    // prl::time(GRID->getCenter()*1.5);
    // cout << "T RANK : "<<GRID->getCenter() << endl;
    // prl::PrintRowMatrix(6,7,MPIt);

    cblas_dcopy(n, b, 1, MPIr, 1);        // r_0 = b (i.e. b)

    MPIImposeBC(MPIr,GRID);
    
    // MPI_Barrier(MPI_COMM_WORLD);
    // prl::time(GRID->getCenter()*1.5);
    // cout << "R RANK : "<<GRID->getCenter() << endl;
    // prl::PrintRowMatrix(6,7,MPIr);

    cblas_daxpy(n, -1.0, MPIt, 1, MPIr, 1);

    // MPI_Barrier(MPI_COMM_WORLD);
    // prl::time(GRID->getCenter()*1.5);
    // cout << "Z RANK : "<<GRID->getCenter() << endl;
    // prl::PrintRowMatrix(6,7,MPIz);
    
    /// solve for outer walls
    MPIPrecondition(MPIr, MPIz,GRID);
    
    cblas_dcopy(n, MPIz, 1, MPIp, 1);        // p_0 = r_0

    // MPI_Barrier(MPI_COMM_WORLD);
    // prl::time(GRID->getCenter()*1.5);
    // cout << "P RANK : "<<GRID->getCenter() << endl;
    // prl::PrintRowMatrix(6,7,MPIp);

    k = 0;
    /// Using conjugate gradient method to solve for new vorticity
    do {
        k++;
        // Perform action of Nabla^2 * p

        MPIApplyOperator(MPIp, MPIt,GRID);
        // if(k==1){ MPI_Barrier(MPI_COMM_WORLD);
        // prl::time(GRID->getCenter()*1.5);
        // cout << "T RANK : "<<GRID->getCenter() << endl;
        // prl::PrintRowMatrix(6,7,MPIt);}

        localphaden = cblas_ddot(n, MPIt, 1, MPIp, 1);  // alpha = p_k^T A p_k
        MPI_Allreduce(&localphaden,&alphaden,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
        localphanum = cblas_ddot(n, MPIr, 1, MPIz, 1); // compute alpha_k
        MPI_Allreduce(&localphanum,&alphanum,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
        alpha = alphanum/alphaden;
        // if(GRID->getCenter()==0&&k==1)cout << "MPI alpha: "<<alpha << endl;

        locbetaden  = cblas_ddot(n, MPIr, 1, MPIz, 1);  // z_k^T r_k
        MPI_Allreduce(&locbetaden,&betaden,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

        cblas_daxpy(n,  alpha, MPIp, 1, x, 1);  // x_{k+1} = x_k + alpha_k p_k
        cblas_daxpy(n, -alpha, MPIt, 1, MPIr, 1); // r_{k+1} = r_k - alpha_k A p_k

        loceps=0.0;
        GRID->edgeZero(MPIr);
    loceps = cblas_dnrm2(n, MPIr, 1);
        MPI_Allreduce(&loceps,&eps,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
        // eps = sqrt(eps);

        if (eps < tol*tol) {
            // cout << "ITerations: "<<k << endl;
            // cout << "alpha: "<<alpha << endl;
            break;
        }
        MPIPrecondition(MPIr, MPIz,GRID);
        locbetanum = cblas_ddot(n, MPIr, 1, MPIz, 1);
        MPI_Allreduce(&locbetanum,&betanum,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
        beta = betanum/betaden;
        // if(GRID->getCenter()==0&&k==1)cout << "MPI beta: "<<beta << endl;

        cblas_dcopy(n, MPIz, 1, MPIt, 1);
        cblas_daxpy(n, beta, MPIp, 1, MPIt, 1);
        cblas_dcopy(n, MPIt, 1, MPIp, 1);

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
void SolverCG::ApplyOperator(double* in, double* out) {
    // Assume ordered with y-direction fastest (column-by-column)
    double dx2i = 1.0/dx/dx;
    double dy2i = 1.0/dy/dy;
    int jm1 = 0, jp1 = 2;
    /// Central finite difference scheme to calculate Nabla^2
    for (int j = 1; j < Ny - 1; ++j) {
        for (int i = 1; i < Nx - 1; ++i) {
            out[IDX(i,j)] = ( -     in[IDX(i-1, j)]
                              + 2.0*in[IDX(i,   j)]
                              -     in[IDX(i+1, j)])*dx2i
                          + ( -     in[IDX(i, jm1)]
                              + 2.0*in[IDX(i,   j)]
                              -     in[IDX(i, jp1)])*dy2i;
        }
        jm1++;
        jp1++;
    }
}
void SolverCG::MPIApplyOperator(double* in, double* out,prl::gridData* GRID) {
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
    // prl::debug(GRID->getCenter(),"at Wall TBLR %d%d%d%d",wallTop,wallBottom,wallLeft,wallRight);
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
void SolverCG::Precondition(double* in, double* out) {
    int i, j;
    double dx2i = 1.0/dx/dx;
    double dy2i = 1.0/dy/dy;
    double factor = 2.0*(dx2i + dy2i);
    for (i = 1; i < Nx - 1; ++i) {
        for (j = 1; j < Ny - 1; ++j) {
            out[IDX(i,j)] = in[IDX(i,j)]/factor;
        }
    }
    // Boundaries
    for (i = 0; i < Nx; ++i) {
        out[IDX(i, 0)] = in[IDX(i,0)];
        out[IDX(i, Ny-1)] = in[IDX(i, Ny-1)];
    }

    for (j = 0; j < Ny; ++j) {
        out[IDX(0, j)] = in[IDX(0, j)];
        out[IDX(Nx - 1, j)] = in[IDX(Nx - 1, j)];
    }
}
void SolverCG::MPIPrecondition(double* in, double* out,prl::gridData* GRID) {
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
    // GRID->exchangeGhost(in);
    for (int j = (!wallTop ? 0 : 1); j < (!wallBottom ? Chunky : Chunky - 1); ++j)
    {
        for (int i = (!wallLeft ? 0 : 1); i < (!wallRight ? Chunkx : Chunkx - 1); ++i)
        {

            out[LOCIDX(i, j)] = in[LOCIDX(i, j)] /factor;
        }
    }
    if (wallTop)
    {
        for (int i = 0; i < Chunkx; ++i)
        {
            out[LOCIDX(i, 0)] = in[LOCIDX(i, 0)];
        }
    }
    if (wallBottom)
    {
        for (int i = 0; i < Chunkx; ++i)
        {
            out[LOCIDX(i, Chunky - 1)] = in[LOCIDX(i, Chunky - 1)];
        }
    }
    if (wallLeft)
    {
        for (int j = 0; j < Chunky; ++j)
        {
            out[LOCIDX(0, j)] = in[LOCIDX(0,j)];
        }
    }
    if (wallRight)
    {
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
void SolverCG::ImposeBC(double* inout) {
        // Boundaries
    for (int i = 0; i < Nx; ++i) {
        inout[IDX(i, 0)] = 0.0;
        inout[IDX(i, Ny-1)] = 0.0;
    }

    for (int j = 0; j < Ny; ++j) {
        inout[IDX(0, j)] = 0.0;
        inout[IDX(Nx - 1, j)] = 0.0;
    }

}
void SolverCG::MPIImposeBC(double* inout,prl::gridData* GRID) {
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
        for (int i = 0; i < Chunkx; ++i)
        {
            inout[LOCIDX(i, 0)] = 0.0;
        }
    }
    if (wallBottom)
    {
        for (int i = 0; i < Chunkx; ++i)
        {
            inout[LOCIDX(i, Chunky - 1)] = 0.0;
        }
    }
    if (wallLeft)
    {
        for (int j = 0; j < Chunky; ++j)
        {
            inout[LOCIDX(0, j)] = 0.0;
        }
    }
    if (wallRight)
    {
        for (int j = 0; j < Chunky; ++j)
        {
            inout[LOCIDX(Chunkx - 1, j)] = 0.0;
        }
    }
    delete[] start;
    delete[] stop;
}
