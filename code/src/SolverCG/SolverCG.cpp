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
#include <cmath>
#include <unistd.h>
#include <omp.h>
#include <cblas.h>
#include "../../include/SolverCG.h"
#include "../../include/ParallelFunc.h"
using namespace std;

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

    loceps = cblas_ddot(n, b, 1,b,1);
    MPI_Allreduce(&loceps,&eps,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
    eps=sqrt(eps);
    if (eps < tol*tol) {
        std::fill(x, x+n, 0.0);
        if(GRID->getCenter()==0)cout << "Norm is " << eps << endl;
        return;
    }

    /// Initialise conjugate gradient algorithm with correct states and boundary conditions.
    ApplyOperator(x, t,GRID);

    cblas_dcopy(n, b, 1, r, 1);        // r_0 = b (i.e. b)

    ImposeBC(r,GRID);
    
    cblas_daxpy(n, -1.0, t, 1, r, 1);

    
    /// solve for outer walls
    Precondition(r, z,GRID);
    
    cblas_dcopy(n, z, 1, p, 1);        // p_0 = r_0


    k = 0;
    /// Using conjugate gradient method to solve for new vorticity
    do {
        k++;
        /// Perform action of Nabla^2 * p

        ApplyOperator(p, t,GRID);

        localphaden = 0.0;
        localphaden = cblas_ddot(n, t, 1, p, 1);  // alpha = p_k^T A p_k
        MPI_Allreduce(&localphaden,&alphaden,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

        localphanum = 0.0;
        localphanum = cblas_ddot(n, r, 1, z, 1); // compute alpha_k
        MPI_Allreduce(&localphanum,&alphanum,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

        alpha = alphanum/alphaden;
        locbetaden = 0.0;
        locbetaden  = cblas_ddot(n, r, 1, z, 1);  // z_k^T r_k
        MPI_Allreduce(&locbetaden,&betaden,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);

        cblas_daxpy(n,  alpha, p, 1, x, 1);  // x_{k+1} = x_k + alpha_k p_k

        cblas_daxpy(n, -alpha, t, 1, r, 1); // r_{k+1} = r_k - alpha_k A p_k

        loceps=0.0;
        GRID->edgeZero(r);
        loceps = cblas_ddot(n, r, 1,r,1);
        MPI_Allreduce(&loceps,&eps,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
        eps=sqrt(eps);

        if (eps < tol*tol) {
            break;
        }
        Precondition(r, z,GRID);

        locbetanum = 0.0;
        locbetanum = cblas_ddot(n, r, 1, z, 1);
        MPI_Allreduce(&locbetanum,&betanum,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
        beta = betanum/betaden;

        cblas_dcopy(n, z, 1, t, 1);

        cblas_daxpy(n, beta, p, 1, t, 1);

        cblas_dcopy(n, t, 1, p, 1);

    } while (k < 5000); // Set a maximum number of iterations

    if (k == 5000) {
        if(GRID->getCenter()==0)cout << "FAILED TO CONVERGE" << endl;
        exit(-1);
    }

    if(GRID->getCenter()==0)cout << "Converged in " << k << " iterations. eps = " << eps << endl;
}

void SolverCG::ApplyOperator(double* in, double* out,prl::gridData* GRID) {
    double dx2i = 1.0/dx/dx;
    double dy2i = 1.0/dy/dy;
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    GRID->exchangeGhost(in);
    bool wallTop = GRID->getUp()<0;
    bool wallBottom = GRID->getDown()<0;
    bool wallLeft = GRID->getLeft()<0;
    bool wallRight =GRID->getRight()<0;
    int startY = (!wallTop ? 0 : 1);
    int startX = (!wallLeft ? 0 : 1);
    int stopY = (!wallBottom ? Chunky : Chunky - 1);
    int stopX = (!wallRight ? Chunkx : Chunkx - 1);
    int N = stopX-startX;
    double *tmpROW =nullptr;

    #pragma omp parallel for schedule(static) private(tmpROW)
    for (int j = startY; j < stopY; ++j)
    {
        tmpROW = new double[N]();
        /**
         * (-  in[LOCIDX(i - 1, j)]
         *  +  2.0 * in[LOCIDX(i, j)]
         *  -  in[LOCIDX(i + 1, j)]) * dx2i 
         * 
         */
        cblas_dscal(N, 0.0, &out[LOCIDX(startX+0,j+0)], 1);
        cblas_daxpy(N,-1.0,&in[LOCIDX(startX-1,j+0)],1,tmpROW,1);
        cblas_daxpy(N,+2.0,&in[LOCIDX(startX+0,j+0)],1,tmpROW,1);
        cblas_daxpy(N,-1.0,&in[LOCIDX(startX+1,j+0)],1,tmpROW,1);
        cblas_dscal(N,dx2i,tmpROW,1);

        /**
         * (  -  in[LOCIDX(i, j-1)]
         *     +  2.0 * in[LOCIDX(i, j)]
         *     -  in[LOCIDX(i, j+1)]) * dy2i;
         * 
         */
        cblas_daxpy(N,-1.0,&in[LOCIDX(startX+0,j-1)],1,&out[LOCIDX(startX+0,j+0)],1);
        cblas_daxpy(N,+2.0,&in[LOCIDX(startX+0,j+0)],1,&out[LOCIDX(startX+0,j+0)],1);
        cblas_daxpy(N,-1.0,&in[LOCIDX(startX+0,j+1)],1,&out[LOCIDX(startX+0,j+0)],1);
        cblas_dscal(N,dy2i,&out[LOCIDX(startX+0,j+0)],1);

        ///out[LOCIDX(i, j)] = (-  in[LOCIDX(i - 1, j)]+  2.0 * in[LOCIDX(i, j)]-  in[LOCIDX(i + 1, j)]) * dx2i + (  -  in[LOCIDX(i, j-1)]+  2.0 * in[LOCIDX(i, j)]-  in[LOCIDX(i, j+1)]) * dy2i;
        cblas_daxpy(N,1.0,tmpROW,1,&out[LOCIDX(startX+0,j+0)],1);
        delete[] tmpROW;
    }
}

void SolverCG::Precondition(double* in, double* out,prl::gridData* GRID) {
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    bool wallTop = GRID->getUp()<0;
    bool wallBottom = GRID->getDown()<0;
    bool wallLeft = GRID->getLeft()<0;
    bool wallRight =GRID->getRight()<0;
    double dx2i = 1.0/dx/dx;
    double dy2i = 1.0/dy/dy;
    double factor = 2.0*(dx2i + dy2i);
    int startY = (!wallTop ? 0 : 1);
    int startX = (!wallLeft ? 0 : 1);
    int stopY = (!wallBottom ? Chunky : Chunky - 1);
    int stopX = (!wallRight ? Chunkx : Chunkx - 1);

    #pragma omp parallel for schedule(static)
    for (int j = startY; j < stopY; ++j)
    {
        /**
        *   #pragma omp parallel for schedule(static) 
        *   for (int j = (!wallTop ? 0 : 1); j < (!wallBottom ? Chunky : Chunky - 1); ++j)
        *   {
        *       for (int i = (!wallLeft ? 0 : 1); i < (!wallRight ? Chunkx : Chunkx - 1); ++i)
        *       {
        *   
        *           out[LOCIDX(i, j)] = in[LOCIDX(i, j)] /factor;
        *       }
        *   } 
        */
        cblas_dcopy(stopX-startX,&in[LOCIDX(startX, j)],1,&out[LOCIDX(startX, j)],1);
        cblas_dscal(stopX-startX,1.0/factor,&out[LOCIDX(startX, j)],1);
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
}
void SolverCG::ImposeBC(double* inout,prl::gridData* GRID) {
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    bool wallTop = GRID->getUp()<0;
    bool wallBottom = GRID->getDown()<0;
    bool wallLeft = GRID->getLeft()<0;
    bool wallRight =GRID->getRight()<0;
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
}
