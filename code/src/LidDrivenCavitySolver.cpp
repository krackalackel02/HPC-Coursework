/**
 * @file LidDrivenCavitySolver.cpp
 * @author Paul Kallarackel
 * @brief This is the main solver file which uses methods defined in LidDrivenCavity.h and SolverCG.h
 * @version 0.1
 * @date 2024-02-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <iostream>
#include <mpi.h>

using namespace std;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "../include/LidDrivenCavity.h"
#include "../include/ParallelFunc.h"
#include "../include/Test.h"
#include <cmath>

int main(int argc, char *argv[])
{
    int world_rank = 0;
    int world_size = 0;

    // Initialise MPI.
    int err = MPI_Init(&argc, &argv);
    if (err != MPI_SUCCESS)
    {
        cout << "ERROR initialising MPI" << endl;
        return 1;
    }

    // Get the rank and comm size on each process.
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 1  || world_size>16)
    {

        if( world_rank==0)cout << "ERROR use between 1 and 16 process" << endl;
        MPI_Finalize();
        return 1;
    }
    if (sqrt(world_size)!=floor(sqrt(world_size)))
    {

        if( world_rank==0)cout << "ERROR use square number process" << endl;
        MPI_Finalize();
        return 1;
    }
    MPI_Comm cartComm;
    int Nx,Ny;
    double Lx,Ly,dt,T,Re;
    int world_p = (int)sqrt(world_size);
    if(world_rank==0)cout << "P: "<<world_p << endl;
    const int dims = 2;
    int sizes[dims] = {world_p,world_p};
    int periods[dims] = {0, 0};
    int reorder = 0;

    MPI_Cart_create(MPI_COMM_WORLD, dims, sizes, periods, reorder, &cartComm);



        /// Allows custom simulation
        po::options_description opts(
            "Solver for the 2D lid-driven cavity incompressible flow problem");
        opts.add_options()
            ("Lx",  po::value<double>()->default_value(1.0),
                    "Length of the domain in the x-direction.")
            ("Ly",  po::value<double>()->default_value(1.0),
                    "Length of the domain in the y-direction.")
            ("Nx",  po::value<int>()->default_value(9),
                    "Number of grid points in x-direction.")
            ("Ny",  po::value<int>()->default_value(9),
                    "Number of grid points in y-direction.")
            ("dt",  po::value<double>()->default_value(0.01),
                    "Time step size.")
            ("T",   po::value<double>()->default_value(1),
                    "Final time.")
            ("Re",  po::value<double>()->default_value(10),
                    "Reynolds number.")
            ("verbose",    "Be more verbose.")
            ("help",       "Print help message.");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, opts), vm);
        po::notify(vm);

        Nx = vm["Nx"].as<int>();
        Ny = vm["Ny"].as<int>();
        Lx = vm["Lx"].as<double>();
        Ly = vm["Ly"].as<double>();
        dt = vm["dt"].as<double>();
        T = vm["T"].as<double>();
        Re = vm["Re"].as<double>();
        
    if (world_p>Nx||world_p>Ny)
    {

        if( world_rank==0)cout << "ERROR use Nx and Ny greater than P" << endl;
        MPI_Finalize();
        return 1;
    }
        if (vm.count("help")) {
            cout << opts << endl;
            return 0;
        }

    

    // prl::gridData GRID = prl::gridData( Nx,  Ny,  world_p,  world_rank,  cartComm);
    // int* start = new int[2]();
    // int* stop = new int[2]();
    // int* cartCoord = new int[2]();
    // GRID.getCartCoord(cartCoord);
    // GRID.getStart(start);
    // GRID.getStop(stop);
    // prl::debug(world_rank,"CARTCOORD: (%2d,%2d)\n",cartCoord[0],cartCoord[1]);
    // prl::debug(world_rank,"START: (%2d,%2d)\n",start[0],start[1]);
    // prl::debug(world_rank,"STOP: (%2d,%2d)\n",stop[0],stop[1]);
    // MPI_Barrier(MPI_COMM_WORLD);
    // prl::debug(world_rank, "   %2d\n", GRID.getUp());
    // prl::debug(world_rank, "%2d %2d %2d\n", GRID.getLeft(), GRID.getCenter(), GRID.getRight());
    // prl::debug(world_rank, "   %2d\n", GRID.getDown());


    
        /// New solver instance for defined problem above
        LidDrivenCavity* solver = new LidDrivenCavity();
        solver->SetDomainSize(Lx, Ly);
        solver->SetGridSize(Nx,Ny);
        solver->SetTimeStep(dt);
        solver->SetFinalTime(T);
        solver->SetReynoldsNumber(Re);
        // MPI_Barrier(MPI_COMM_WORLD);

        if(world_rank==0)solver->PrintConfiguration();

        solver->MPIInitialise(world_p,world_rank,cartComm);
        if(world_rank==0)solver->WriteSolution("output/ic.txt");

        solver->MPIIntegrate();

        if(world_rank==0)solver->WriteSolution("output/final.txt");
        solver->MPIWriteSolution("output/final.txt");

        delete solver;
    

    // delete[] start;
    // delete[] stop;
    // delete[] cartCoord;
    MPI_Finalize();
	return 0;
}