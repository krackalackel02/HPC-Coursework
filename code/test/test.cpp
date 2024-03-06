#include "../include/LidDrivenCavity.h"
#include "../include/SolverCG.h"
#include <cmath>
#include <cblas.h>
#include <iostream>
#define IDX(I, J) ((J) * Nx + (I))

#define BOOST_TEST_MODULE Solver_and_Cavity_Test_Suite

// ... BEFORE including the Boost test header
#include <boost/test/included/unit_test.hpp>

// Function to compare numerical and analytical solutions (if available)
bool compareSolution(const double *numerical, const double *analytical, int size, double tol = 1e-3)
{
    double *ERR = new double[size];
    cblas_dcopy(size, analytical, 1, ERR, 1);
    cblas_daxpy(size, -1.0, numerical, 1, ERR, 1);
    double e = cblas_dnrm2(size, ERR, 1);
    cout << "ERROR: " << e << endl;
    delete[] ERR;
    if(e<tol)cout << "Test Pass!" << endl;
    return e < tol;
}

BOOST_AUTO_TEST_CASE(SolverCGAnalayticalTest)
{
    int Nx = 2e3; // Example grid size
    int Ny = 2e3; // Example grid size
    double Lx = 1.0; // Example grid size
    double Ly = 1.0; // Example grid size
    double dx = Lx / (double)(Nx - 1);
    double dy = Ly / (double)(Ny - 1);
    int N = Nx * Ny;

    /// Analytical Solution
    cout << "\nSolverCG Analaytical Test:" << endl;
    SolverCG *solver = new SolverCG(Nx, Ny, dx, dy);
    double *solution = new double[N];
    double *analytical = new double[N];
    double *v = new double[N];
    const double k = 2.0 / Lx;
    const double l = 2.0 / Ly;
    for (int i = 0; i < Nx; ++i)
    {
        for (int j = 0; j < Ny; ++j)
        {
            v[IDX(i, j)] = -M_PI * M_PI * (k * k + l * l) * sin(M_PI * k * i * dx) * sin(M_PI * l * j * dy);
        }
    }
    for (int i = 0; i < Nx; ++i)
    {
        for (int j = 0; j < Ny; ++j)
        {
            solution[IDX(i, j)] = -sin(M_PI * k * i * dx) * sin(M_PI * l * j * dy);
        }
    }
    solver->Solve(v, analytical);
    BOOST_CHECK(compareSolution(solution, analytical, N));

    delete solver;
    delete[] solution;
    delete[] analytical;
    delete[] v;
}

BOOST_AUTO_TEST_CASE(SolverCGZeroTest){
    /// Zero input solution
    cout << "\nSolverCG Zero Test:" << endl;
    int Nx = 1e2; // Example grid size
    int Ny = 1e2; // Example grid size
    double Lx = 1.0; // Example grid size
    double Ly = 1.0; // Example grid size
    double dx = Lx / (double)(Nx - 1);
    double dy = Ly / (double)(Ny - 1);
    int N = Nx * Ny;
    SolverCG *smallSolver = new SolverCG(Nx, Ny, dx, dy);
    double* smallSolution = new double[N]{};
    double* smallAnalytical = new double[N];
    double* smallV = new double[N]{1e-8};
    smallSolver->Solve(smallV, smallAnalytical);
    BOOST_CHECK(compareSolution(smallSolution, smallAnalytical, N));
    delete smallSolver;
    delete[] smallSolution;
    delete[] smallAnalytical;
    delete[] smallV;
}
