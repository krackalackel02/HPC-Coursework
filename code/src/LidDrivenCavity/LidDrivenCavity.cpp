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
void LidDrivenCavity::Initialise()
{
    CleanUp();
    /// Init vorticity, stream function and temporary vector for simulation
    v = new double[Npts]();
    vnew = new double[Npts]();
    s = new double[Npts]();
    tmp = new double[Npts]();
    /// Init new solver with domain size and discretisation
    cg = new SolverCG(Nx, Ny, dx, dy);
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
        // std::cout << "Step: " << setw(8) << t
        //   << "  Time: " << setw(8) << t * dt
        //   << std::endl;
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
    for (int i = 1; i < Nx - 1; ++i)
    {
        for (int j = 1; j < Ny - 1; ++j)
        {
            /// x velocity: u = d(psi)/dy
            u0[IDX(i, j)] = (s[IDX(i, j + 1)] - s[IDX(i, j)]) / dy;
            /// y velocity: -v = d(psi)/dx
            u1[IDX(i, j)] = -(s[IDX(i + 1, j)] - s[IDX(i, j)]) / dx;
        }
    }
    for (int i = 0; i < Nx; ++i)
    {
        /// freestream velocity
        u0[IDX(i, Ny - 1)] = U;
    }

    std::ofstream f(file.c_str());
    std::cout << "Writing file " << file << std::endl;
    int k = 0;
    for (int i = 0; i < Nx; ++i)
    {
        for (int j = 0; j < Ny; ++j)
        {
            /// array index of coordinates
            k = IDX(i, j);
            /// printing: |x|y|omega|psi|xu|v|
            f << i * dx << " " << j * dy << " " << v[k] << " " << s[k]
              << " " << u0[k] << " " << u1[k] << std::endl;
        }
        f << std::endl;
    }
    f.close();

    // free-up memory
    delete[] u0;
    delete[] u1;
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

    // Boundary node vorticity - Obtained using equations 6-9 on "brief.pdf"
    for (int i = 1; i < Nx - 1; ++i)
    {
        // top
        v[IDX(i, 0)] = 2.0 * dy2i * (s[IDX(i, 0)] - s[IDX(i, 1)]);
        // bottom
        v[IDX(i, Ny - 1)] = 2.0 * dy2i * (s[IDX(i, Ny - 1)] - s[IDX(i, Ny - 2)]) - 2.0 * dyi * U;
    }
    for (int j = 1; j < Ny - 1; ++j)
    {
        // left
        v[IDX(0, j)] = 2.0 * dx2i * (s[IDX(0, j)] - s[IDX(1, j)]);
        // right
        v[IDX(Nx - 1, j)] = 2.0 * dx2i * (s[IDX(Nx - 1, j)] - s[IDX(Nx - 2, j)]);
    }

    // Compute interior vorticity - Obtained using equations 10 on "brief.pdf"
    for (int i = 1; i < Nx - 1; ++i)
    {
        for (int j = 1; j < Ny - 1; ++j)
        {
            v[IDX(i, j)] = dx2i * (2.0 * s[IDX(i, j)] - s[IDX(i + 1, j)] - s[IDX(i - 1, j)]) + 1.0 / dy / dy * (2.0 * s[IDX(i, j)] - s[IDX(i, j + 1)] - s[IDX(i, j - 1)]);
        }
    }

    // Time advance vorticity - Obtained using equations 11 on "brief.pdf"
    for (int i = 1; i < Nx - 1; ++i)
    {
        for (int j = 1; j < Ny - 1; ++j)
        {
            vnew[IDX(i, j)] = v[IDX(i, j)] + dt * (((s[IDX(i + 1, j)] - s[IDX(i - 1, j)]) * 0.5 * dxi * (v[IDX(i, j + 1)] - v[IDX(i, j - 1)]) * 0.5 * dyi) - ((s[IDX(i, j + 1)] - s[IDX(i, j - 1)]) * 0.5 * dyi * (v[IDX(i + 1, j)] - v[IDX(i - 1, j)]) * 0.5 * dxi) + nu * (v[IDX(i + 1, j)] - 2.0 * v[IDX(i, j)] + v[IDX(i - 1, j)]) * dx2i + nu * (v[IDX(i, j + 1)] - 2.0 * v[IDX(i, j)] + v[IDX(i, j - 1)]) * dy2i);
        }
    }

    // // Sinusoidal test case with analytical solution, which can be used to test
    // // the Poisson solver

    /* const int k = 3;
    const int l = 3;
    for (int i = 0; i < Nx; ++i) {
        for (int j = 0; j < Ny; ++j) {
            vnew[IDX(i,j)] = -M_PI * M_PI * (k * k + l * l)
                                       * sin(M_PI * k * i * dx)
                                       * sin(M_PI * l * j * dy);
        }
    } */

    // Solve Poisson problem
    cg->Solve(vnew, s);
}
void LidDrivenCavity::MPIAdvance()
{
    /// divisors to be used and multiplied by to optimise code
    double dxi = 1.0 / dx;
    double dyi = 1.0 / dy;
    double dx2i = 1.0 / dx / dx;
    double dy2i = 1.0 / dy / dy;
    int* start = new int[2];
    int* stop = new int[2];
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    GRID->getStart(start);
    GRID->getStop(stop);
    GRID->exchangeGhost(MPIs);
    bool wallTop = start[1]==0;
    bool wallBottom = stop[1]==Ny-1;
    bool wallLeft = start[0]==0;
    bool wallRight = stop[0]==Nx-1;
    // Boundary node vorticity - Obtained using equations 6-9 on "brief.pdf"
        if(wallTop){
            for (int i=0; i < Chunkx; ++i) {
                MPIv[LOCIDX(i,0)] = 2.0 * dy2i * (MPIs[LOCIDX(i, 0)] - MPIs[LOCIDX(i, 1)]);
        }
        }
        if(wallBottom){
            for (int i=0; i < Chunkx; ++i) {
                MPIv[LOCIDX(i, Chunky-1)] = 2.0 * dy2i * (MPIs[LOCIDX(i, Chunky - 1)] - MPIs[LOCIDX(i, Chunky - 2)]) - 2.0 * dyi * U;
        }
        }
        if(wallLeft){
            for (int j=0; j < Chunky; ++j) {
                MPIv[LOCIDX(0, j)] = 2.0 * dx2i * (MPIs[LOCIDX(0, j)] - MPIs[LOCIDX(1, j)]);
        }
        }
        if(wallRight){
            for (int j=0; j < Chunky; ++j) {
                MPIv[LOCIDX(Chunkx - 1, j)] = 2.0 * dx2i * (MPIs[LOCIDX(Chunkx - 1, j)] - MPIs[LOCIDX(Chunkx - 2, j)]);
        }
        }
        GRID->exchangeGhost(MPIv);
        // Compute interior vorticity - Obtained using equations 10 on "brief.pdf"
        for (int i = (!wallLeft?0:1); i < (!wallRight?Chunkx:Chunkx-1) ; ++i)
        {
            for (int j = (!wallTop?0:1); j < (!wallBottom?Chunky:Chunky-1) ; ++j)
            {
                MPIv[LOCIDX(i, j)] = dx2i * (2.0 * MPIs[LOCIDX(i, j)] - MPIs[LOCIDX(i + 1, j)] - MPIs[LOCIDX(i - 1, j)]) + 1.0 / dy / dy * (2.0 * MPIs[LOCIDX(i, j)] - MPIs[LOCIDX(i, j + 1)] - MPIs[LOCIDX(i, j - 1)]);
            }
        }
        GRID->exchangeGhost(MPIv);
        // // Time advance vorticity - Obtained using equations 11 on "brief.pdf"
        for (int i = (!wallLeft?0:1); i < (!wallRight?Chunkx:Chunkx-1) ; ++i)
        {
            for (int j = (!wallTop?0:1); j < (!wallBottom?Chunky:Chunky-1) ; ++j)
            {
                MPIv[LOCIDX(i, j)] = MPIv[LOCIDX(i, j)] + dt * (((MPIs[LOCIDX(i + 1, j)] - MPIs[LOCIDX(i - 1, j)]) * 0.5 * dxi * (MPIv[LOCIDX(i, j + 1)] - MPIv[LOCIDX(i, j - 1)]) * 0.5 * dyi) - ((MPIs[LOCIDX(i, j + 1)] - MPIs[LOCIDX(i, j - 1)]) * 0.5 * dyi * (MPIv[LOCIDX(i + 1, j)] - MPIv[LOCIDX(i - 1, j)]) * 0.5 * dxi) + nu * (MPIv[LOCIDX(i + 1, j)] - 2.0 * MPIv[LOCIDX(i, j)] + MPIv[LOCIDX(i - 1, j)]) * dx2i + nu * (MPIv[LOCIDX(i, j + 1)] - 2.0 * MPIv[LOCIDX(i, j)] + MPIv[LOCIDX(i, j - 1)]) * dy2i);
            }
        }
    delete[] start;
    delete[] stop;
    // cg->MPISolve(MPIv,MPIs);
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
    // prl::debug(rank, "CARTCOORD: (%2d,%2d)\n", cartCoord[0], cartCoord[1]);
    // prl::debug(rank, "START: (%2d,%2d)\n", start[0], start[1]);
    // prl::debug(rank, "STOP: (%2d,%2d)\n", stop[0], stop[1]);
    // MPI_Barrier(MPI_COMM_WORLD);
    // prl::debug(rank, "   %2d\n", GRID->getUp());
    // prl::debug(rank, "%2d %2d %2d\n", GRID->getLeft(), GRID->getCenter(), GRID->getRight());
    // prl::debug(rank, "   %2d\n", GRID->getDown());
    int size = (GRID->getChunky() + 2) * (GRID->getChunkx() + 2);
    double fill = 0.0;
    if(rank==1||rank==p)fill = 1.0;
    MPIv = new double[size]();
    MPItmp = new double[size]();
    MPIs = new double[size]();
    for (int i=0; i < size; ++i) {
    
        MPIv[i]+=fill;
        MPIs[i]+=fill;
        MPItmp[i]+=fill;
    
    }
    if(rank==0)prl::PrintRowMatrix(GRID->getChunkx()+2,GRID->getChunky()+2,MPIv);
    GRID->exchangeGhost(MPIv,{{"msg", "Checking init exchg"}});
    if(rank==0)prl::PrintRowMatrix(GRID->getChunkx()+2,GRID->getChunky()+2,MPIv);
    for (int i=0; i < size; ++i) {
    
        MPIv[i]+=fill;
        MPIs[i]+=fill;
        MPItmp[i]+=fill;
    
    }
    GRID->exchangeGhost(MPIv,{{"msg", "Checking init exchg"}});
    if(rank==0)prl::PrintRowMatrix(GRID->getChunkx()+2,GRID->getChunky()+2,MPIv);
    MPIAdvance();
}

/**
 * @brief Frees up allocated memory for simulation data
 *
 */
void LidDrivenCavity::CleanUp()
{
    /// if v has been initialised e.g. nolonger nullptr free up all the memory as solver has been called
    if (v)
    {
        delete[] v;
        delete[] s;
        delete[] tmp;
        delete cg;
    }

    if (GRID)
    {

        delete GRID;
        cout << "about to delete GRID" << endl;
        // Stop double deletion
        GRID = nullptr;
        delete[] MPIv;
        delete[] MPIs;
        delete[] MPItmp;
    }
}