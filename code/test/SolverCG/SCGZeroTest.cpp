/**
 * @file SCGZeroTest.cpp
 * @author Paul Kallarackel (paulkallarackel@gmail.com)
 * @brief Contains unit test for zero input vorticity
 * @version 0.1
 * @date 2024-03-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "../../include/SolverCG.h"
#include "../../include/LidDrivenCavity.h"
#include "../../include/MyTest.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <cmath>


BOOST_AUTO_TEST_CASE(SolverCG_SOLVE_ZeroTest){
    /// sync up ranks
    MPI_Barrier(MPI_COMM_WORLD);
    /// Initialise cart grid and rank info
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
    LidDrivenCavity* ldc = new LidDrivenCavity();
    ldc->SetDomainSize(Lx, Ly);
    ldc->SetGridSize(Nx,Ny);
    ldc->Initialise(world_p,world_rank,cartComm); /// set vorticity to 0

    /// extract grid data fro local error in chunk
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
    solver->Solve(locV,solution,GRID);

    /// calc and reduce local error
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
    MPI_Reduce(&locError,&ERR,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    if(world_rank==0){
        ERR = sqrt(ERR);
        BOOST_CHECK(myTest::compare(ERR,0.001));
    }

    /// free up resources
    delete solver;
    delete ldc;
    delete[] solution;
    delete[] analytical;
    delete[] locV;
    delete[] start;
    delete[] stop;
    MPI_Comm_free(&cartComm);
    MPI_Barrier(MPI_COMM_WORLD);
}