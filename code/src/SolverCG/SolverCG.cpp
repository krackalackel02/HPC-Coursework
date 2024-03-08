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

#include <cblas.h>

#include "../../include/SolverCG.h"
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

    /// Initialise conjugate gradient algorithm with correct states and boundary conditions.
    ApplyOperator(x, t);
    cblas_dcopy(n, b, 1, r, 1);        // r_0 = b (i.e. b)
    ImposeBC(r);

    cblas_daxpy(n, -1.0, t, 1, r, 1);
    /// solve for outer walls
    Precondition(r, z);
    cblas_dcopy(n, z, 1, p, 1);        // p_0 = r_0

    k = 0;
    /// Using conjugate gradient method to solve for new vorticity
    do {
        k++;
        // Perform action of Nabla^2 * p
        ApplyOperator(p, t);

        alpha = cblas_ddot(n, t, 1, p, 1);  // alpha = p_k^T A p_k
        alpha = cblas_ddot(n, r, 1, z, 1) / alpha; // compute alpha_k
        beta  = cblas_ddot(n, r, 1, z, 1);  // z_k^T r_k

        cblas_daxpy(n,  alpha, p, 1, x, 1);  // x_{k+1} = x_k + alpha_k p_k
        cblas_daxpy(n, -alpha, t, 1, r, 1); // r_{k+1} = r_k - alpha_k A p_k

        eps = cblas_dnrm2(n, r, 1);

        if (eps < tol*tol) {
            break;
        }
        Precondition(r, z);
        beta = cblas_ddot(n, r, 1, z, 1) / beta;

        cblas_dcopy(n, z, 1, t, 1);
        cblas_daxpy(n, beta, p, 1, t, 1);
        cblas_dcopy(n, t, 1, p, 1);

    } while (k < 5000); // Set a maximum number of iterations

    if (k == 5000) {
        cout << "FAILED TO CONVERGE" << endl;
        exit(-1);
    }

    cout << "Converged in " << k << " iterations. eps = " << eps << endl;
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
