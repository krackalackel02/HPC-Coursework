/**
 * @file LidDrivenCavity.cpp
 * @author Paul Kallarackel
 * @brief This file implements the methods found in LidDrivenCavity.h
 * @version 0.1
 * @date 2024-02-28
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <cmath>
#include "../../include/ParallelFunc.h"
#include <mpi.h>

using namespace std;
#include <sstream>

#include <cblas.h>
/**
 * @brief Used as shorthand for coordinates
 *
 */
#define IDX(I, J) ((J) * Nx + (I))

#include "../../include/LidDrivenCavity.h"
#include "../../include/SolverCG.h"

/**
 * @brief Construct a new Lid Driven Cavity:: Lid Driven Cavity object
 *
 */
LidDrivenCavity::LidDrivenCavity()
{
}
/**
 * @brief Destroy the Lid Driven Cavity:: Lid Driven Cavity object
 *
 */
LidDrivenCavity::~LidDrivenCavity()
{
    CleanUp();
}
prl::gridData *LidDrivenCavity::getGRID()
{
    return GRID;
}
/**
 * @brief Sets width and height of cavity
 *
 * @param xlen Domain of cavity across x
 * @param ylen Domain of cavity across y
 */
void LidDrivenCavity::SetDomainSize(double xlen, double ylen)
{
    this->Lx = xlen;
    this->Ly = ylen;
    UpdateDxDy();
}
void LidDrivenCavity::Setv(int idx, double val)
{
    v[idx] = val;
}
double LidDrivenCavity::Getv(int idx)
{
    return v[idx];
}
void LidDrivenCavity::Sets(int idx, double val)
{
    s[idx] = val;
}
double LidDrivenCavity::Gets(int idx)
{
    return s[idx];
}
/**
 * @brief Discretises Domain into a grid
 *
 * @param nx Number of points across x dimension
 * @param ny Number fo points across y dimension
 */
void LidDrivenCavity::SetGridSize(int nx, int ny)
{
    this->Nx = nx;
    this->Ny = ny;
    UpdateDxDy();
}
/**
 * @brief Sets timestep for solver
 *
 * @param deltat size of times tep
 */
void LidDrivenCavity::SetTimeStep(double deltat)
{
    this->dt = deltat;
}
/**
 * @brief Sets final time of solver
 *
 * @param finalt final time for simulation
 */
void LidDrivenCavity::SetFinalTime(double finalt)
{
    this->T = finalt;
}
/**
 * @brief Sets Reynolds number of problem
 *
 * @param re Reynolds number
 */
void LidDrivenCavity::SetReynoldsNumber(double re)
{
    this->Re = re;
    this->nu = 1.0 / re;
}
/**
 * @brief Initialises spatial solver for cavity
 *
 */
void LidDrivenCavity::Initialise(int p, int rank, MPI_Comm comm)
{
    CleanUp();
    /// Init vorticity, stream function and temporary vector for simulation
    CartInit(p, rank, comm);
    /// Init new solver with domain size and discretisation
    cg = new SolverCG(Nx, Ny, dx, dy, GRID);
}
/**
 * @brief Calls solver and advances simulation for each timestep
 *
 */
void LidDrivenCavity::Integrate()
{
    /// number of timesteps
    int NSteps = ceil(T / dt);
    for (int t = 0; t < NSteps; ++t)
    {
        if (GRID->getCenter() == 0)
            std::cout << "Step: " << setw(8) << t
                      << "  Time: " << setw(8) << t * dt
                      << std::endl;
        Advance();
    }
}
/**
 * @brief Writes out simulation data to output file with tabulated properties |x|y|omega|psi|xu|v|
 *
 * @param file Output file name
 */
void LidDrivenCavity::WriteSolution(std::string file)
{
    double *u0 = new double[Nx * Ny]();
    double *u1 = new double[Nx * Ny]();
    int *start = new int[2];
    int *stop = new int[2];
    int *cartCoord = new int[2];
    int *currCoord = new int[2];
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    GRID->getStart(start);
    GRID->getStop(stop);
    GRID->getCartCoord(cartCoord);
    bool wallTop = start[1] == 0;
    bool wallBottom = stop[1] == Ny - 1;
    bool wallLeft = start[0] == 0;
    bool wallRight = stop[0] == Nx - 1;
    int size = (Chunkx + 2) * (Chunky + 2);
    double *MPIu0 = new double[size]();
    double *MPIu1 = new double[size]();
    GRID->exchangeGhost(s);
    #pragma omp parallel for schedule(static) 
    for (int j = (!wallTop ? 0 : 1); j < (!wallBottom ? Chunky : Chunky - 1); ++j)
    {
        for (int i = (!wallLeft ? 0 : 1); i < (!wallRight ? Chunkx : Chunkx - 1); ++i)
        {

            /// x velocity: u = d(psi)/dy
            MPIu0[LOCIDX(i, j)] = (s[LOCIDX(i, j - 1)] - s[LOCIDX(i, j)]) / dy;
            /// y velocity: -v = d(psi)/dx
            MPIu1[LOCIDX(i, j)] = -(s[LOCIDX(i + 1, j)] - s[LOCIDX(i, j)]) / dx;
        }
    }
    if (wallTop)
    {
        #pragma omp parallel for schedule(static) 
        for (int i = 0; i < Chunkx; ++i)
        {
            /// freestream velocity
            MPIu0[LOCIDX(i, 0)] = U;
        }
    }
    int k = 0;
    double x, y;
    std::ostringstream oss;
    oss << file << "";
    file = oss.str();
    if (GRID->getCenter() == 0)
    {
        std::cout << "Writing file " << file << std::endl;
        std::ofstream f(file.c_str());
        f.close();
    }
    std::stringstream ss; // stringstream to collect data
    for (int i = 0; i < Nx; ++i)
    {
        for (int j = Ny - 1; j >= 0; --j)
        {
            MPI_Barrier(MPI_COMM_WORLD);
            if (start[0] <= i && stop[0] >= i && start[1] <= j && stop[1] >= j)
            {
                std::ofstream f(file.c_str(), std::ios::app);
                ss.str(""); // Clear the contents of the stringstream
                ss.clear(); // Reset any error flags
                currCoord[0] = i;
                currCoord[1] = j;
                GRID->glo2loc(currCoord, currCoord);
                // array index of coordinates
                k = LOCIDX(currCoord[0], currCoord[1]);
                x = i * dx;
                y = (Ny - 1 - j) * dy;
                // Collecting data into stringstream
                ss << x << " " << y << " " << v[k] << " " << s[k] << " " << MPIu0[k] << " " << MPIu1[k] << std::endl;
                f << ss.str();
                f.close();
            }
        }
        if (GRID->getCenter() == 0)
        {
            std::ofstream f(file.c_str(), std::ios::app);
            f << std::endl; // Add a newline after each row for clarity
            f.close();
        }
    }
    delete[] u0;
    delete[] u1;
    delete[] MPIu0;
    delete[] MPIu1;
    delete[] start;
    delete[] stop;
    delete[] cartCoord;
    delete[] currCoord;
}
/**
 * @brief Prints simulation results to terminal
 *
 */
void LidDrivenCavity::PrintConfiguration()
{
    cout << "Grid size: " << Nx << " x " << Ny << endl;
    cout << "Spacing:   " << dx << " x " << dy << endl;
    cout << "Length:    " << Lx << " x " << Ly << endl;
    cout << "Grid pts:  " << Npts << endl;
    cout << "Timestep:  " << dt << endl;
    cout << "Steps:     " << ceil(T / dt) << endl;
    cout << "Reynolds number: " << Re << endl;
    cout << "Linear solver: preconditioned conjugate gradient" << endl;
    cout << endl;
    if (nu * dt / dx / dy > 0.25)
    {
        cout << "ERROR: Time-step restriction not satisfied!" << endl;
        cout << "Maximum time-step is " << 0.25 * dx * dy / nu << endl;
        exit(-1);
    }
}

/**
 * @brief Calculates space domain steps and total number of grid points
 *
 */
void LidDrivenCavity::UpdateDxDy()
{
    /// calculating grid size and number of poitns
    dx = Lx / (Nx - 1);
    dy = Ly / (Ny - 1);
    Npts = Nx * Ny;
}

/**
 * @brief Central finite difference scheme to update grid with values at next timestep
 *
 */
void LidDrivenCavity::Advance()
{
    /// divisors to be used and multiplied by to optimise code
    double dxi = 1.0 / dx;
    double dyi = 1.0 / dy;
    double dx2i = 1.0 / dx / dx;
    double dy2i = 1.0 / dy / dy;
    int *start = new int[2];
    int *stop = new int[2];
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    GRID->getStart(start);
    GRID->getStop(stop);
    GRID->exchangeGhost(s);
    bool wallTop = start[1] == 0;
    bool wallBottom = stop[1] == Ny - 1;
    bool wallLeft = start[0] == 0;
    bool wallRight = stop[0] == Nx - 1;
    // Boundary node vorticity - Obtained using equations 6-9 on "brief.pdf"
    if (wallTop)
    {
        #pragma omp parallel for schedule(static)
        for (int i = (!wallLeft ? 0 : 1); i < (!wallRight ? Chunkx : Chunkx - 1); ++i)
        {
            v[LOCIDX(i, 0)] = 2.0 * dy2i * (s[LOCIDX(i, 0)] - s[LOCIDX(i, 1)]) - 2.0 * dyi * U;
        }
    }
    if (wallBottom)
    {
        #pragma omp parallel for schedule(static)
        for (int i = (!wallLeft ? 0 : 1); i < (!wallRight ? Chunkx : Chunkx - 1); ++i)
        {
            v[LOCIDX(i, Chunky - 1)] = 2.0 * dy2i * (s[LOCIDX(i, Chunky - 1)] - s[LOCIDX(i, Chunky - 2)]);
        }
    }
    if (wallLeft)
    {
        #pragma omp parallel for schedule(static)
        for (int j = (!wallTop ? 0 : 1); j < (!wallBottom ? Chunky : Chunky - 1); ++j)
        {
            v[LOCIDX(0, j)] = 2.0 * dx2i * (s[LOCIDX(0, j)] - s[LOCIDX(1, j)]);
        }
    }
    if (wallRight)
    {
        #pragma omp parallel for schedule(static)
        for (int j = (!wallTop ? 0 : 1); j < (!wallBottom ? Chunky : Chunky - 1); ++j)
        {
            v[LOCIDX(Chunkx - 1, j)] = 2.0 * dx2i * (s[LOCIDX(Chunkx - 1, j)] - s[LOCIDX(Chunkx - 2, j)]);
        }
    }
    // Compute interior vorticity - Obtained using equations 10 on "brief.pdf"
    #pragma omp parallel for schedule(static) collapse(2)
    for (int i = (!wallLeft ? 0 : 1); i < (!wallRight ? Chunkx : Chunkx - 1); ++i)
    {
        for (int j = (!wallTop ? 0 : 1); j < (!wallBottom ? Chunky : Chunky - 1); ++j)
        {
            v[LOCIDX(i, j)] = dx2i * (2.0 * s[LOCIDX(i, j)] - s[LOCIDX(i + 1, j)] - s[LOCIDX(i - 1, j)]) + 1.0 / dy / dy * (2.0 * s[LOCIDX(i, j)] - s[LOCIDX(i, j - 1)] - s[LOCIDX(i, j + 1)]);
        }
    }
    GRID->exchangeGhost(v);
    // Time advance vorticity - Obtained using equations 11 on "brief.pdf"
    #pragma omp parallel for schedule(static) collapse(2)
    for (int i = (!wallLeft ? 0 : 1); i < (!wallRight ? Chunkx : Chunkx - 1); ++i)
    {
        for (int j = (!wallTop ? 0 : 1); j < (!wallBottom ? Chunky : Chunky - 1); ++j)
        {
            vnew[LOCIDX(i, j)] = v[LOCIDX(i, j)] + dt * (((s[LOCIDX(i + 1, j)] - s[LOCIDX(i - 1, j)]) * 0.5 * dxi * (v[LOCIDX(i, j - 1)] - v[LOCIDX(i, j + 1)]) * 0.5 * dyi) - ((s[LOCIDX(i, j - 1)] - s[LOCIDX(i, j + 1)]) * 0.5 * dyi * (v[LOCIDX(i + 1, j)] - v[LOCIDX(i - 1, j)]) * 0.5 * dxi) + nu * (v[LOCIDX(i + 1, j)] - 2.0 * v[LOCIDX(i, j)] + v[LOCIDX(i - 1, j)]) * dx2i + nu * (v[LOCIDX(i, j - 1)] - 2.0 * v[LOCIDX(i, j)] + v[LOCIDX(i, j + 1)]) * dy2i);
        }
    }
    cg->Solve(vnew, s, GRID);
    delete[] start;
    delete[] stop;
}
void LidDrivenCavity::CartInit(int p, int rank, MPI_Comm comm)
{
    GRID = new prl::gridData(Nx, Ny, p, rank, comm);
    int *start = new int[2]();
    int *stop = new int[2]();
    int *cartCoord = new int[2]();
    GRID->getCartCoord(cartCoord);
    GRID->getStart(start);
    GRID->getStop(stop);
    int size = (GRID->getChunky() + 2) * (GRID->getChunkx() + 2);
    v = new double[size]();
    vnew = new double[size]();
    tmp = new double[size]();
    s = new double[size]();
}

/**
 * @brief Frees up allocated memory for simulation data
 *
 */
void LidDrivenCavity::CleanUp()
{
    /// if v has been initialised e.g. nolonger nullptr free up all the memory as solver has been called
    if (GRID)
    {

        delete GRID;
        // Stop double deletion
        GRID = nullptr;
        delete[] v;
        delete[] vnew;
        delete[] s;
        delete[] tmp;
    }
}