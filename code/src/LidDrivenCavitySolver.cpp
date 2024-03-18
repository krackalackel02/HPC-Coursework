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
#include "../include/LidDrivenCavity.h"
#include <cmath>
#include <boost/program_options.hpp>
#include <boost/timer/timer.hpp>
#include <omp.h>

using namespace std;
namespace po = boost::program_options;
double measureExecutionTime(boost::timer::cpu_timer& timer, std::function<void()> function) {
    timer.start();
    function();
    timer.stop();
    return timer.elapsed().wall * 1e-9; // Convert to seconds
}
int main(int argc, char *argv[])
{
    // Start the timer
    boost::timer::cpu_timer timer;
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

    if (world_size < 1 )
    {

        if( world_rank==0)cout << "ERROR use at least 1 proccess" << endl;
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
    int Nx,Ny,ThreadNum,ThreadID;
    double Lx,Ly,dt,T,Re;
    int world_p = (int)sqrt(world_size);
    if(world_rank==0)cout << "P: "<<world_p << endl;
    const int dims = 2;
    int sizes[dims] = {world_p,world_p};
    int periods[dims] = {0, 0};
    int reorder = 0;

    MPI_Cart_create(MPI_COMM_WORLD, dims, sizes, periods, reorder, &cartComm);
	
	char *env_var = getenv("OMP_NUM_THREADS");
    if (env_var != NULL) {
        ThreadNum = atoi(env_var); // Convert string to integer
    } else {
		ThreadNum=1;
        printf("OMP_NUM_THREADS is not set. Using default number of threads.\nSet env variable with \"export OMP_NUM_THREADS=4\"\n");
    }

    /// Allows custom simulation
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
    po::store(po::parse_command_line(argc, argv, opts), vm);
    po::notify(vm);

    Nx = vm["Nx"].as<int>();
    Ny = vm["Ny"].as<int>();
    Lx = vm["Lx"].as<double>();
    Ly = vm["Ly"].as<double>();
    dt = vm["dt"].as<double>();
    T = vm["T"].as<double>();
    Re = vm["Re"].as<double>();
    
    omp_set_num_threads(std::min(ThreadNum,omp_get_max_threads()));
        
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
	} // All threads join master thread and terminate

    /// New solver instance for defined problem above
    LidDrivenCavity* solver = new LidDrivenCavity();
    solver->SetDomainSize(Lx, Ly);
    solver->SetGridSize(Nx,Ny);
    solver->SetTimeStep(dt);
    solver->SetFinalTime(T);
    solver->SetReynoldsNumber(Re);

    if(world_rank==0)solver->PrintConfiguration();

    double initTimeSeconds = measureExecutionTime(timer, [&]() { solver->Initialise(world_p, world_rank, cartComm); });
    double writeICTimeSeconds = measureExecutionTime(timer, [&]() { solver->WriteSolution("output/ic.txt"); });
    double integTimeSeconds = measureExecutionTime(timer, [&]() { solver->Integrate(); });
    double writeSOLTimeSeconds = measureExecutionTime(timer, [&]() { solver->WriteSolution("output/final.txt"); });

    if(world_rank==0)printf("Initialising: %6fs WriteIC: %6fs Integrate: %6fs WriteSOL: %6fs\n",initTimeSeconds,writeICTimeSeconds,integTimeSeconds,writeSOLTimeSeconds);
    if(world_rank==0)printf("Total: %6fs\n",(initTimeSeconds+writeICTimeSeconds+integTimeSeconds+writeSOLTimeSeconds));

    delete solver;
    MPI_Comm_free(&cartComm);
    MPI_Finalize();
	return 0;
}