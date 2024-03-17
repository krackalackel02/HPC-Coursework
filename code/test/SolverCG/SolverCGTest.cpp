#include "../../include/SolverCG.h"
#include "../../include/LidDrivenCavity.h"
#include "../../include/Test.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <cmath>

BOOST_AUTO_TEST_SUITE(SolverCGTestSuite)


BOOST_AUTO_TEST_CASE(SolverCG_SOLVE_AnalayticalTest)
{
    int world_size,world_rank;
    MPI_Comm cartComm;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_p = (int)sqrt(world_size);
    if (world_rank == 0)
        std::cout << "P: " << world_p << std::endl;
    const int dims = 2;
    int sizes[dims] = {world_p, world_p};
    int periods[dims] = {0, 0};
    int reorder = 0;
    MPI_Cart_create(MPI_COMM_WORLD, dims, sizes, periods, reorder, &cartComm);

    int Nx = 2e3; // Example grid size
    int Ny = 2e3; // Example grid size
    double Lx = 1.0; // Example grid size
    double Ly = 1.0; // Example grid size
    double dx = Lx / (double)(Nx - 1);
    double dy = Ly / (double)(Ny - 1);
    int N = Nx * Ny;
    LidDrivenCavity* ldc = new LidDrivenCavity();
    ldc->SetDomainSize(Lx, Ly);
    ldc->SetGridSize(Nx,Ny);
    ldc->MPIInitialise(world_p,world_rank,cartComm);
    /// Analytical Solution
    std::cout << "\nSolverCG Analaytical Test:" << std::endl;
    prl::gridData* GRID= ldc->getGRID();
    SolverCG *solver = new SolverCG(Nx, Ny, dx, dy,GRID);
    double *V = new double[N];
    int *start = new int[2];
    int *stop = new int[2];
    int *currCoord = new int[2];
    GRID->getStart(start);
    GRID->getStop(stop);
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    int size = (Chunkx+2)*(Chunky+2);
    double *analytical = new double[size]();
    double *solution = new double[size]();
    double *locV = new double[size]();
    const double k = 2.0 / Lx;
    const double l = 2.0 / Ly;
    int idx;
    double x,y;
    for (int i = start[0]; i <= stop[0]; ++i)
    {
        for (int j = start[1]; j <= stop[1]; ++j)
        {
                idx = LOCIDX(i-start[0],j-start[1]);
                x = i*dx;
                y = (Ny-1-j)*dy;
                locV[idx]= M_PI * M_PI * (k * k + l * l) * sin(M_PI * k * x) * sin(M_PI * l * y );
                analytical[idx] = sin(M_PI * k * x) * sin(M_PI * l * y);
        }
    }
    cout << "Starting Solve..." << endl;
    MPI_Barrier(MPI_COMM_WORLD);
    solver->MPISolve(locV,solution,GRID);
    cout << "Finishing Solve..." << endl;
    double locError = 0.0;
    double ERR = 0.0;
    for (int i = start[0]; i <= stop[0]; ++i)
    {
        for (int j = start[1]; j <= stop[1]; ++j)
        {
                idx = LOCIDX(i-start[0],j-start[1]);
                locError+= (analytical[idx]-solution[idx])*(analytical[idx]-solution[idx]);
        }
    }
    MPI_Allreduce(&locError,&ERR,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
    
    // int start = 10000;
    // for (int i=start; i < start+100; ++i) {
    
    //     std::cout << "ANALYTICAL: "<<analytical[i]<<" Solution: "<<analytical[i] << std::endl;
    
    // }
    // cout << "ERROR->"<<ERR << endl;
    BOOST_CHECK(compare(ERR,100));

    delete solver;
    delete ldc;
    delete[] solution;
    delete[] analytical;
    delete[] V;
    delete[] start;
    delete[] stop;
    delete[] currCoord;
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