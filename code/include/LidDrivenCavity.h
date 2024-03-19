/**
 * @file LidDrivenCavity.h
 * @author Paul Kallarackel (paulkallarackel@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-03-06
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include "./ParallelFunc.h"
#include <mpi.h>
#include <string>
using namespace std;

class SolverCG;

/**
 * @class LidDrivenCavity
 * @brief This class sets up the initial conditions and solves the simulation
 *
 * This class sets the initial conditions and type of problem with its domain and gridsize.
 * It then makes an instance of the solver found in "SolverCG.h".
 * After it integrates through each time step and calles the solve method to calculate the properties of the cavity for each time step.
 */
class LidDrivenCavity
{
public:

    /**
     * @brief Function Instiates global grid props and neighbours with prl::gridData object
     * 
     * @param p Number of ranks per row and column
     * @param rank Current ranks number
     * @param comm Cartesian communicator
     * This function allows each rank to know its global position, neighbours with prl::gridData constructor
     */
    void CartInit(int p, int rank, MPI_Comm &comm);

    /**
     * @brief Construct a new Lid Driven Cavity object
     *
     */
    LidDrivenCavity();

    /**
     * @brief Destroy the Lid Driven Cavity object
     *
     */
    ~LidDrivenCavity();

    /**
     * @brief Set the Domain Size object
     *
     * @param xlen Length of x domain
     * @param ylen Length of y domain
     */
    void SetDomainSize(double xlen, double ylen);

    /**
     * @brief Set the Grid Size object
     *
     * @param nx Number of points in x discretisation
     * @param ny Number of points in y discretisation
     */
    void SetGridSize(int nx, int ny);

    /**
     * @brief Set the Time Step object
     *
     * @param deltat Time step
     */
    void SetTimeStep(double deltat);

    /**
     * @brief Set the Final Time object
     *
     * @param finalt Final time
     */
    void SetFinalTime(double finalt);

    /**
     * @brief Set the Reynolds Number object
     *
     * @param Re Reynolds number
     */
    void SetReynoldsNumber(double Re);

    /**
     * @brief Initialises Solver with domain and cart grid and initial values
     * 
     * @param p Square root of total numbe rof ranks
     * @param rank Current rank
     * @param comm cartesian communicator
     */
    void Initialise(int p, int rank, MPI_Comm &comm);

    /**
     * @brief Used to iterate through time steps can call Advance()
     *
     */
    void Integrate();

    /**
     * @brief Writes solution to output file
     *
     * @param file File name
     */
    void WriteSolution(std::string file);

    /**
     * @brief Prints simulation data to the terminal
     *
     */
    void PrintConfiguration();

    /**
     * @brief This function gets the private prl::gridData object to use for testing to access private rank data like Chunk size
     * 
     * @return prl::gridData* This object is used to access global properties of rank in defined grid
     */
    prl::gridData *getGRID();

private:
    double *v = nullptr;    ///< Vorticity vector
    double *vnew = nullptr; ///< New Vorticity vector
    double *s = nullptr;    ///< Stream function vector
    prl::gridData *GRID = nullptr; ///< Grid object with neighbour and chunk size props
    double dt = 0.01;       ///< Time step
    double T = 1.0;         ///< Final Time
    double dx;              ///< X step
    double dy;              ///< Y step
    int Nx = 9;             ///< Number of x  points
    int Ny = 9;             ///< Number of y points
    int Npts = 81;          ///< Total number of points
    double Lx = 1.0;        ///< Length of x domain
    double Ly = 1.0;        ///< Length of y domain
    double Re = 10;         ///< Reynolds number
    double U = 1.0;         ///< Freestream velocity
    double nu = 0.1;        ///< Nu : 1/Re
    SolverCG *cg = nullptr; ///< Spatial solver

    /**
     * @brief Cleans up and deallocated dynamic memory
     *
     */
    void CleanUp();

    /**
     * @brief Updated step size and number of points
     *
     */
    void UpdateDxDy();

    /**
     * @brief Advances simulation by one time step
     *
     */
    void Advance();
};
