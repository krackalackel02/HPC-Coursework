/**
 * @file LidDrivenCavitySolver.cpp
 * @author Paul Kallarackel
 * @brief This is the main LDC file which uses methods defined in LidDrivenCavity.h and SolverCG.h
 * @version 0.1
 * @date 2024-02-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <iostream> /// used for output
#include <mpi.h> /// used for Distributed Parallel Programming
#include "../include/LidDrivenCavity.h" /// used for LDC class
#include <cmath> /// used for basic math op.
#include <boost/program_options.hpp> /// used for reading terminal options
#include <boost/timer/timer.hpp> /// used to time code
#include <omp.h> /// used for Shared Memory Parallel Programming

using namespace std;
namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    /// Initialise MPI.
    int err = MPI_Init(&argc, &argv);
    if (err != MPI_SUCCESS)
    {
        cout << "ERROR initialising MPI" << endl;
        return 1;
    }

    /// Get the rank and comm size on each process.
    int world_rank;
    int world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    /// Check more than one rank
    if (world_size < 1 )
    {
        if( world_rank==0)cout << "ERROR use at least 1 proccess" << endl;
        MPI_Finalize();
        return 1;
    }
    /// Check square number of ranks
    if (sqrt(world_size)!=floor(sqrt(world_size)))
    {
        if( world_rank==0)cout << "ERROR use square number process" << endl;
        MPI_Finalize();
        return 1;
    }
    /// print out p used in P=p^2 ranks
    int world_p = (int)sqrt(world_size);
    if(world_rank==0)cout << "p: "<<world_p << endl;
    

    MPI_Comm cartComm;
    int Nx,Ny,ThreadID;
    double Lx,Ly,dt,T,Re;
    const int dims = 2;
    int sizes[dims] = {world_p,world_p};
    int periods[dims] = {0, 0};
    int reorder = 0;
    MPI_Cart_create(MPI_COMM_WORLD, dims, sizes, periods, reorder, &cartComm);
	
    /// Retrieve OMP Thread Number variable with fallback of Serial Case (1)
    int ThreadNum;
	char *env_var = getenv("OMP_NUM_THREADS");
    if (env_var != NULL) {
        ThreadNum = atoi(env_var);
    } else {
		ThreadNum=1;
        if(world_rank==0)printf("OMP_NUM_THREADS is not set.\nUsing default serial (1) number of threads.\nSet env variable with \"export OMP_NUM_THREADS=4\"\n");
    }
    /// Allows for program to be executed even if env variable is larger than max threads available
    omp_set_num_threads(std::min(ThreadNum,omp_get_max_threads()));

    /// makes options map to check against command line options
    po::options_description opts(
        "Solver for the 2D lid-driven cavity incompressible flow problem");
    opts.add_options()
        ("Lx",  po::value<double>()->default_value(1.0),
                "Length of the domain in the x-direction.")
        ("Ly",  po::value<double>()->default_value(1.0),
                "Length of the domain in the y-direction.")
        ("Nx",  po::value<int>()->default_value(8),
                "Number of grid points in x-direction.")
        ("Ny",  po::value<int>()->default_value(10),
                "Number of grid points in y-direction.")
        ("dt",  po::value<double>()->default_value(0.01),
                "Time step size.")
        ("T",   po::value<double>()->default_value(10),
                "Final time.")
        ("Re",  po::value<double>()->default_value(10),
                "Reynolds number.")
        ("verbose",    "Be more verbose.")
        ("help",       "Print help message.");
    po::variables_map vm;
    /// reads options from terminal
    po::store(po::parse_command_line(argc, argv, opts), vm);
    po::notify(vm);
    /// Evaluate terminal options onto associated variables
    Nx = vm["Nx"].as<int>();
    Ny = vm["Ny"].as<int>();
    Lx = vm["Lx"].as<double>();
    Ly = vm["Ly"].as<double>();
    dt = vm["dt"].as<double>();
    T = vm["T"].as<double>();
    Re = vm["Re"].as<double>();
    
    /// Reads out available options to user
    if (vm.count("help")) {
            cout << opts << endl;
            return 0;
        }
    /// Ensures each Rank will have at least one column and row of  global grid
    if (world_p>Nx||world_p>Ny)
    {
        if( world_rank==0)cout << "ERROR use Nx and Ny greater than P" << endl;
        MPI_Finalize();
        return 1;
    }

    /// Paralel section to read out set number of threads
	#pragma omp parallel private(ThreadID)
	{
		// Obtain and print thread id
		ThreadID = omp_get_thread_num();
		// Only master thread does this
		if (ThreadID == 0&&world_rank==0)
		{
			ThreadNum = omp_get_num_threads();
			cout << "Number of threads = " << ThreadNum << endl;
		}
	}

    /// Make Timer
    boost::timer::cpu_timer timer;

    /// New LDC instance for defined problem above with setters
    LidDrivenCavity* LDC = new LidDrivenCavity();
    LDC->SetDomainSize(Lx, Ly);
    LDC->SetGridSize(Nx,Ny);
    LDC->SetTimeStep(dt);
    LDC->SetFinalTime(T);
    LDC->SetReynoldsNumber(Re);

    /// Print to terminal once
    if(world_rank==0)LDC->PrintConfiguration();

    /// Reset Timer
    timer.start();
    /// Initialise each ranks local vectors and prl::gridData object
    LDC->Initialise(world_p, world_rank, cartComm);
    /// Calc elpased time in seconds
    double initTimeSeconds = timer.elapsed().wall * 1e-9;

    timer.start();
    /// Prints Initial conditions to .txt
    LDC->WriteSolution("output/ic.txt");
    double writeICTimeSeconds = timer.elapsed().wall * 1e-9;

    timer.start();
    /// Advances and solves spatial problem at each time step until final time reached
    LDC->Integrate();
    double integTimeSeconds = timer.elapsed().wall * 1e-9;

    timer.start();
    /// Prints Final conditions to .txt
    LDC->WriteSolution("output/final.txt");
    double writeSOLTimeSeconds = timer.elapsed().wall * 1e-9;

    /// Prints timing info to terminal and txt file 
    if(world_rank==0)printf("Initialising: %6fs WriteIC: %6fs Integrate: %6fs WriteSOL: %6fs\n",initTimeSeconds,writeICTimeSeconds,integTimeSeconds,writeSOLTimeSeconds);
    if(world_rank==0)printf("Total: %6fs\n",(initTimeSeconds+writeICTimeSeconds+integTimeSeconds+writeSOLTimeSeconds));
    if (world_rank == 0) {
        FILE *fp;
        fp = fopen("output/timings.txt", "a+");
        if (fp == NULL) {
            printf("Error opening file!\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        fprintf(fp, "%-13d |  %-13d |  %10.6f        |  %11.6f  |  %11.6f  |  %11.6f  |  %11.6f\n",
                world_size,ThreadNum,initTimeSeconds, writeICTimeSeconds, integTimeSeconds, writeSOLTimeSeconds,
                initTimeSeconds + writeICTimeSeconds + integTimeSeconds + writeSOLTimeSeconds);

        fclose(fp);
    }


    /// free ldc object and MPI comm
    delete LDC;
    MPI_Comm_free(&cartComm);
    MPI_Finalize();
	return 0;
}