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
using namespace std;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "../include/LidDrivenCavity.h"

int main(int argc, char **argv)
{
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
        ("T",   po::value<double>()->default_value(1.0),
                 "Final time.")
        ("Re",  po::value<double>()->default_value(10),
                 "Reynolds number.")
        ("verbose",    "Be more verbose.")
        ("help",       "Print help message.");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, opts), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << opts << endl;
        return 0;
    }

    /// New solver instance for defined problem above
    LidDrivenCavity* solver = new LidDrivenCavity();
    solver->SetDomainSize(vm["Lx"].as<double>(), vm["Ly"].as<double>());
    solver->SetGridSize(vm["Nx"].as<int>(),vm["Ny"].as<int>());
    solver->SetTimeStep(vm["dt"].as<double>());
    solver->SetFinalTime(vm["T"].as<double>());
    solver->SetReynoldsNumber(vm["Re"].as<double>());

    solver->PrintConfiguration();

    solver->Initialise();

    solver->WriteSolution("output/ic.txt");

    solver->Integrate();

    solver->WriteSolution("output/final.txt");

	return 0;
}