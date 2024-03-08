#include "../../include/SolverCG.h"
#include "../../include/Test.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <cmath>


BOOST_AUTO_TEST_SUITE(SolverCGTestSuite)


BOOST_AUTO_TEST_CASE(SolverCG_SOLVE_AnalayticalTest)
{
    int Nx = 2e3; // Example grid size
    int Ny = 2e3; // Example grid size
    double Lx = 1.0; // Example grid size
    double Ly = 1.0; // Example grid size
    double dx = Lx / (double)(Nx - 1);
    double dy = Ly / (double)(Ny - 1);
    int N = Nx * Ny;

    /// Analytical Solution
    std::cout << "\nSolverCG Analaytical Test:" << std::endl;
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
            v[IDX(i, j)] = M_PI * M_PI * (k * k + l * l) * sin(M_PI * k * i * dx) * sin(M_PI * l * j * dy);
        }
    }
    for (int i = 0; i < Nx; ++i)
    {
        for (int j = 0; j < Ny; ++j)
        {
            analytical[IDX(i, j)] = sin(M_PI * k * i * dx) * sin(M_PI * l * j * dy);
        }
    }
    solver->Solve(v, solution);
    // int start = 10000;
    // for (int i=start; i < start+100; ++i) {
    
    //     std::cout << "ANALYTICAL: "<<analytical[i]<<" Solution: "<<analytical[i] << std::endl;
    
    // }
    BOOST_CHECK(compareSolution(analytical, solution, N,1e-3));

    delete solver;
    delete[] solution;
    delete[] analytical;
    delete[] v;
}

BOOST_AUTO_TEST_CASE(SolverCG_SOLVE_ZeroTest){
    /// Zero input solution
    std::cout << "\nSolverCG Zero Test:" << std::endl;
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
    BOOST_CHECK(compareSolution(smallSolution, smallAnalytical, N,1e-3));
    delete smallSolver;
    delete[] smallSolution;
    delete[] smallAnalytical;
    delete[] smallV;
}

BOOST_AUTO_TEST_SUITE_END()