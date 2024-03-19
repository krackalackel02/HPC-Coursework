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
#include "../../include/LidDrivenCavity.h"
#include "../../include/SolverCG.h"

LidDrivenCavity::LidDrivenCavity()
{
}
LidDrivenCavity::~LidDrivenCavity()
{
    CleanUp();
}
prl::gridData *LidDrivenCavity::getGRID()
{
    return GRID;
}
void LidDrivenCavity::SetDomainSize(double xlen, double ylen)
{
    this->Lx = xlen;
    this->Ly = ylen;
    UpdateDxDy();
}
void LidDrivenCavity::SetGridSize(int nx, int ny)
{
    this->Nx = nx;
    this->Ny = ny;
    UpdateDxDy();
}
void LidDrivenCavity::SetTimeStep(double deltat)
{
    this->dt = deltat;
}
void LidDrivenCavity::SetFinalTime(double finalt)
{
    this->T = finalt;
}
void LidDrivenCavity::SetReynoldsNumber(double re)
{
    this->Re = re;
    this->nu = 1.0 / re;
}
void LidDrivenCavity::Initialise(int p, int rank, MPI_Comm &comm)
{
    CleanUp();
    /// Init vorticity, stream function and temporary vector for simulation
    CartInit(p, rank, comm);
    /// Init new solver with domain size and discretisation
    cg = new SolverCG(Nx, Ny, dx, dy, GRID);
}
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
void LidDrivenCavity::WriteSolution(std::string file)
{
    int *start = new int[2];
    int *stop = new int[2];
    int *currCoord = new int[2];
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    GRID->getStart(start);
    GRID->getStop(stop);
    bool wallTop = GRID->getUp()<0;
    bool wallBottom = GRID->getDown()<0;
    bool wallLeft = GRID->getLeft()<0;
    bool wallRight =GRID->getRight()<0;
    int size = (Chunkx + 2) * (Chunky + 2);
    double *u0 = new double[size]();
    double *u1 = new double[size]();
    GRID->exchangeGhost(s);
    #pragma omp parallel for schedule(static) 
    for (int j = (!wallTop ? 0 : 1); j < (!wallBottom ? Chunky : Chunky - 1); ++j)
    {
        for (int i = (!wallLeft ? 0 : 1); i < (!wallRight ? Chunkx : Chunkx - 1); ++i)
        {

            /// x velocity: u = d(psi)/dy
            u0[LOCIDX(i, j)] = (s[LOCIDX(i, j - 1)] - s[LOCIDX(i, j)]) / dy;
            /// y velocity: -v = d(psi)/dx
            u1[LOCIDX(i, j)] = -(s[LOCIDX(i + 1, j)] - s[LOCIDX(i, j)]) / dx;
        }
    }
    if (wallTop)
    {
        #pragma omp parallel for schedule(static) 
        for (int i = 0; i < Chunkx; ++i)
        {
            /// freestream velocity
            u0[LOCIDX(i, 0)] = U;
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
                ss << x << " " << y << " " << v[k] << " " << s[k] << " " << u0[k] << " " << u1[k] << std::endl;
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
    delete[] start;
    delete[] stop;
    delete[] currCoord;
}
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

void LidDrivenCavity::UpdateDxDy()
{
    /// calculating grid size and number of poitns
    dx = Lx / (Nx - 1);
    dy = Ly / (Ny - 1);
    Npts = Nx * Ny;
}

void LidDrivenCavity::Advance()
{
    /// divisors to be used and multiplied by to optimise code
    double dxi = 1.0 / dx;
    double dyi = 1.0 / dy;
    double dx2i = 1.0 / dx / dx;
    double dy2i = 1.0 / dy / dy;
    int Chunkx = GRID->getChunkx();
    int Chunky = GRID->getChunky();
    GRID->exchangeGhost(s);
    bool wallTop = GRID->getUp()<0;
    bool wallBottom = GRID->getDown()<0;
    bool wallLeft = GRID->getLeft()<0;
    bool wallRight =GRID->getRight()<0;
    int startY = (!wallTop ? 0 : 1);
    int startX = (!wallLeft ? 0 : 1);
    int stopY = (!wallBottom ? Chunky : Chunky - 1);
    int stopX = (!wallRight ? Chunkx : Chunkx - 1);
    int N = stopX-startX;
    double* tmp1ROW= nullptr;
    double* tmp2ROW= nullptr;
    double* tmp3ROW= nullptr;
    double* tmp4ROW= nullptr;
    double* tmp5ROW= nullptr;
    double* tmp6ROW= nullptr;
    double* tmp7ROW= nullptr;
    // Boundary node vorticity - Obtained using equations 6-9 on "brief.pdf"
    if (wallTop)
    {
        #pragma omp parallel for schedule(static)
        for (int i = startX; i < stopX; ++i)
        {
            v[LOCIDX(i, 0)] = 2.0 * dy2i * (s[LOCIDX(i, 0)] - s[LOCIDX(i, 1)]) - 2.0 * dyi * U;
        }
    }
    if (wallBottom)
    {
        #pragma omp parallel for schedule(static)
        for (int i = startX; i < stopX; ++i)
        {
            v[LOCIDX(i, Chunky - 1)] = 2.0 * dy2i * (s[LOCIDX(i, Chunky - 1)] - s[LOCIDX(i, Chunky - 2)]);
        }
    }
    if (wallLeft)
    {
        #pragma omp parallel for schedule(static)
        for (int j = startY; j < stopY; ++j)
        {
            v[LOCIDX(0, j)] = 2.0 * dx2i * (s[LOCIDX(0, j)] - s[LOCIDX(1, j)]);
        }
    }
    if (wallRight)
    {
        #pragma omp parallel for schedule(static)
        for (int j = startY; j < stopY; ++j)
        {
            v[LOCIDX(Chunkx - 1, j)] = 2.0 * dx2i * (s[LOCIDX(Chunkx - 1, j)] - s[LOCIDX(Chunkx - 2, j)]);
        }
    }
    // Compute interior vorticity - Obtained using equations 10 on "brief.pdf"
    #pragma omp parallel for schedule(static) private(tmp1ROW,tmp2ROW)
    for (int j = startY; j < stopY; ++j)
    {
        tmp1ROW = new double[N]();
        tmp2ROW = new double[N]();
        cblas_dscal(N, 0.0, &v[LOCIDX(startX+0,j+0)], 1);
        cblas_daxpy(N,-1.0,&s[LOCIDX(startX-1,j+0)],1,tmp1ROW,1);
        cblas_daxpy(N,+2.0,&s[LOCIDX(startX+0,j+0)],1,tmp1ROW,1);
        cblas_daxpy(N,-1.0,&s[LOCIDX(startX+1,j+0)],1,tmp1ROW,1);
        cblas_dscal(N,dx2i,tmp1ROW,1);
    

        cblas_daxpy(N,-1.0,&s[LOCIDX(startX+0,j-1)],1,tmp2ROW,1);
        cblas_daxpy(N,+2.0,&s[LOCIDX(startX+0,j+0)],1,tmp2ROW,1);
        cblas_daxpy(N,-1.0,&s[LOCIDX(startX+0,j+1)],1,tmp2ROW,1);
        cblas_dscal(N,dy2i,tmp2ROW,1);
        cblas_daxpy(N,1.0,tmp1ROW,1,tmp2ROW,1);
        cblas_dcopy(N,tmp2ROW,1,&v[LOCIDX(startX+0,j+0)],1);
        delete[] tmp1ROW;
        delete[] tmp2ROW;
        
    }
    /* #pragma omp parallel for schedule(static) collapse(2)
    for (int j = startY; j < stopY; ++j)
    {
        for (int i = startX; i < stopX; ++i)
        {
            v[LOCIDX(i, j)] = dx2i * (2.0 * s[LOCIDX(i, j)] - s[LOCIDX(i + 1, j)] - s[LOCIDX(i - 1, j)]) ///< term 1
            + 1.0 / dy / dy * (2.0 * s[LOCIDX(i, j)] - s[LOCIDX(i, j - 1)] - s[LOCIDX(i, j + 1)]); ///< term 2
        }
    } */
    
    GRID->exchangeGhost(v);
    // Time advance vorticity - Obtained using equations 11 on "brief.pdf"
    #pragma omp parallel for schedule(static) private(tmp1ROW,tmp2ROW,tmp3ROW,tmp4ROW,tmp5ROW,tmp6ROW,tmp7ROW)
    for (int j = startY; j < stopY; ++j)
    {
        tmp1ROW = new double[N]();
        tmp2ROW = new double[N]();
        tmp3ROW = new double[N]();
        tmp4ROW = new double[N]();
        tmp5ROW = new double[N]();
        tmp6ROW = new double[N]();
        tmp7ROW = new double[N]();

        cblas_dscal(N, 0.0, &vnew[LOCIDX(startX+0,j+0)], 1);

        cblas_daxpy(N,1.0,&v[LOCIDX(startX+1,j+0)],1,tmp5ROW,1);
        cblas_daxpy(N,1.0,&v[LOCIDX(startX-1,j+0)],1,tmp5ROW,1);
        cblas_daxpy(N,-2.0,&v[LOCIDX(startX+0,j+0)],1,tmp5ROW,1);
        cblas_dscal(N,nu*dx2i,tmp5ROW,1);

        cblas_daxpy(N,1.0,&v[LOCIDX(startX+0,j+1)],1,tmp6ROW,1);
        cblas_daxpy(N,1.0,&v[LOCIDX(startX+0,j-1)],1,tmp6ROW,1);
        cblas_daxpy(N,-2.0,&v[LOCIDX(startX+0,j+0)],1,tmp6ROW,1);
        cblas_dscal(N,nu*dy2i,tmp6ROW,1);
        
        cblas_daxpy(N,1.0,tmp5ROW,1,tmp6ROW,1);

        cblas_daxpy(N,1.0,&s[LOCIDX(startX+1,j+0)],1,tmp1ROW,1);
        cblas_daxpy(N,-1.0,&s[LOCIDX(startX-1,j+0)],1,tmp1ROW,1);
        cblas_dscal(N,0.5*dxi,tmp1ROW,1);

        cblas_daxpy(N,1.0,&v[LOCIDX(startX+1,j+0)],1,tmp4ROW,1);
        cblas_daxpy(N,-1.0,&v[LOCIDX(startX-1,j+0)],1,tmp4ROW,1);
        cblas_dscal(N,0.5*dxi,tmp4ROW,1);

        cblas_daxpy(N,1.0,&s[LOCIDX(startX+0,j-1)],1,tmp3ROW,1);
        cblas_daxpy(N,-1.0,&s[LOCIDX(startX+0,j+1)],1,tmp3ROW,1);
        cblas_dscal(N,0.5*dyi,tmp3ROW,1);

        cblas_daxpy(N,1.0,&v[LOCIDX(startX+0,j-1)],1,tmp2ROW,1);
        cblas_daxpy(N,-1.0,&v[LOCIDX(startX+0,j+1)],1,tmp2ROW,1);
        cblas_dscal(N,0.5*dyi,tmp2ROW,1);

        cblas_dsbmv(CblasColMajor,CblasLower,N,0,1.0,tmp3ROW,1,tmp4ROW,1,0.0,tmp5ROW,1);
        cblas_dsbmv(CblasColMajor,CblasLower,N,0,1.0,tmp1ROW,1,tmp2ROW,1,0.0,tmp7ROW,1);
        cblas_daxpy(N,-1.0,tmp5ROW,1,tmp7ROW,1);

        cblas_daxpy(N,1.0,tmp6ROW,1,tmp7ROW,1);

        cblas_daxpy(N,dt,tmp7ROW,1,&vnew[LOCIDX(startX+0,j+0)],1);

        cblas_daxpy(N,1.0,&v[LOCIDX(startX+0,j+0)],1,&vnew[LOCIDX(startX+0,j+0)],1);
        

        delete[] tmp1ROW;
        delete[] tmp2ROW;
        delete[] tmp3ROW;
        delete[] tmp4ROW;
        delete[] tmp5ROW;
        delete[] tmp6ROW;
        delete[] tmp7ROW;
    }


    /* for (int j = startY; j < stopY; ++j){

        for (int i = startX; i < stopX; ++i)
        {
            vnew[LOCIDX(i, j)] = v[LOCIDX(i, j)] + dt * (
                    (
                        (s[LOCIDX(i + 1, j)] - s[LOCIDX(i - 1, j)]) * 0.5 * dxi * ///< Term 1
                        (v[LOCIDX(i, j - 1)] - v[LOCIDX(i, j + 1)]) * 0.5 * dyi ///< Term 2
                    ) 
                    - (
                        (s[LOCIDX(i, j - 1)] - s[LOCIDX(i, j + 1)]) * 0.5 * dyi *  ///< Term 3
                        (v[LOCIDX(i + 1, j)] - v[LOCIDX(i - 1, j)]) * 0.5 * dxi ///< Term 4
                    ) 
                    + nu * (v[LOCIDX(i + 1, j)] - 2.0 * v[LOCIDX(i, j)] + v[LOCIDX(i - 1, j)]) * dx2i  ///< Term 5
                    + nu * (v[LOCIDX(i, j - 1)] - 2.0 * v[LOCIDX(i, j)] + v[LOCIDX(i, j + 1)]) * dy2i ///< Term 6
                );
        }
    } */
    cg->Solve(vnew, s, GRID);
}
void LidDrivenCavity::CartInit(int p, int rank, MPI_Comm &comm)
{
    GRID = new prl::gridData(Nx, Ny, p, rank, comm);
    int size = (GRID->getChunky() + 2) * (GRID->getChunkx() + 2);
    v = new double[size]();
    vnew = new double[size]();
    s = new double[size]();
    bool wallTop = GRID->getUp()<0;
    bool wallBottom = GRID->getDown()<0;
    bool wallLeft = GRID->getLeft()<0;
    bool wallRight =GRID->getRight()<0;
    int startX = (!wallLeft ? 0 : 1);
    int stopX = (!wallRight ? GRID->getChunkx() : GRID->getChunkx() - 1);
    int intRowPoints = stopX-startX;
}

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
    }
}