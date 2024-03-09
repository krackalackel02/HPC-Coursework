#pragma once
#include <iostream>
#include <mpi.h>
// Timer library for debug
#include <chrono>
// Library for sleep()
#include <unistd.h>
namespace prl
{
    struct gridData
    {
    public:
        int offsetX;
        int offsetY;

        // Constructor with initialization list
        gridData(int init_Nx, int init_Ny, int init_world_p, int world_rank, MPI_Comm cartComm);

        // Default destructor
        ~gridData();
        void getCartCoord(int *COORD);
        void getStart(int *COORD);
        void getStop(int *COORD);
        void updateOffset();
        void loc2glo(int *locCoord, int *gloCoord);
        void glo2loc(int *locCoord, int *gloCoord);

    private:
        int Nx;
        int Ny;
        int remx;
        int remy;
        int Chunkx;
        int Chunky;
        int world_rank;
        int world_p;
        int world_size;
        int *cartCoord;
        int *start;
        int *stop;
        int calcOffset(int N, int p, int rank);
        int minChunkSize(int N, int p);
        int rem(int N, int p);
    };
    void debug(int rank, const char *format, ...);
    double get_timer();

}