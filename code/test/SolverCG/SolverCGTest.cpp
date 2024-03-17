#include "../../include/SolverCG.h"
#include "../../include/LidDrivenCavity.h"
#include "../../include/Test.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <cmath>

BOOST_AUTO_TEST_SUITE(SolverCGTestSuite)


BOOST_AUTO_TEST_CASE(SolverCG_SOLVE_AnalayticalTest)
{
    /// Analytical Solution
    int world_size,world_rank;
    MPI_Comm cartComm;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_rank == 0)std::cout << "\nSolverCG Analaytical Test:" << std::endl;
    int world_p = (int)sqrt(world_size);
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
    prl::gridData* GRID= ldc->getGRID();
    SolverCG *solver = new SolverCG(Nx, Ny, dx, dy,GRID);
    int *start = new int[2];
    int *stop = new int[2];
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
    solver->MPISolve(locV,solution,GRID);
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
    ERR = sqrt(ERR);
    if(world_rank==0)BOOST_CHECK(compare(ERR,0.001));

    delete solver;
    delete ldc;
    delete[] solution;
    delete[] analytical;
    delete[] locV;
    delete[] start;
    delete[] stop;
    MPI_Comm_free(&cartComm);
}

BOOST_AUTO_TEST_CASE(SolverCG_SOLVE_ZeroTest){
    /// Zero input solution
    int world_size,world_rank;
    MPI_Comm cartComm;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_rank == 0)std::cout << "\nSolverCG Zero Test:" << std::endl;
    int world_p = (int)sqrt(world_size);
    const int dims = 2;
    int sizes[dims] = {world_p, world_p};
    int periods[dims] = {0, 0};
    int reorder = 0;
    MPI_Cart_create(MPI_COMM_WORLD, dims, sizes, periods, reorder, &cartComm);

    int Nx = 1e2; // Example grid size
    int Ny = 1e2; // Example grid size
    double Lx = 1.0; // Example grid size
    double Ly = 1.0; // Example grid size
    double dx = Lx / (double)(Nx - 1);
    double dy = Ly / (double)(Ny - 1);
    int N = Nx * Ny;
    LidDrivenCavity* ldc = new LidDrivenCavity();
    ldc->SetDomainSize(Lx, Ly);
    ldc->SetGridSize(Nx,Ny);
    ldc->MPIInitialise(world_p,world_rank,cartComm);
    prl::gridData* GRID= ldc->getGRID();
    SolverCG *solver = new SolverCG(Nx, Ny, dx, dy,GRID);
    int *start = new int[2];
    int *stop = new int[2];
    GRID->getStart(start);
    GRID->getStop(stop);
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    int size = (Chunkx+2)*(Chunky+2);
    double *analytical = new double[size]();
    double *solution = new double[size]();
    double *locV = new double[size]();
    solver->MPISolve(locV,solution,GRID);
    int idx;
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
    ERR = sqrt(ERR);
    if(world_rank==0)BOOST_CHECK(compare(ERR,0.001));

    delete solver;
    delete ldc;
    delete[] solution;
    delete[] analytical;
    delete[] locV;
    delete[] start;
    delete[] stop;
    MPI_Comm_free(&cartComm);
}

BOOST_AUTO_TEST_SUITE_END()