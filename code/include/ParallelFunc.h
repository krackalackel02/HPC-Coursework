#pragma once
#include <mpi.h>
#include <string>
#include <unordered_map>
#define EXCHGIDX(I, J) ((J) * (Chunkx+2) + (I))
#define LOCIDX(I, J) EXCHGIDX(I+1,J+1)
namespace prl
{
    struct gridData
    {
    public:
        int offsetX;
        int offsetY;
        MPI_Datatype x_edge_type;
        MPI_Datatype y_edge_type;

        // Constructor with initialization list
        gridData(int init_Nx, int init_Ny, int init_world_p, int init_world_rank, MPI_Comm init_cartComm);
        gridData();

        // Default destructor
        ~gridData();
        void getCartCoord(int *COORD);
        void getStart(int *COORD);
        void getStop(int *COORD);
        int getChunkx();
        int getChunky();
        int getLeft();
        int getCenter();
        int getRight();
        int getUp();
        int getDown();
        void loc2glo(int *locCoord, int *gloCoord);
        void glo2loc(int *locCoord, int *gloCoord);
        void exchangeGhost(double *data,const std::unordered_map<std::string, std::string>& options={});

    private:
        int Nx;
        int Ny;
        int remx;
        int remy;
        int Chunkx;
        int Chunky;
        int world_rank;
        int cartRank;
        int world_p;
        int world_size;
        int *cartCoord;
        int *start;
        int *stop;
        int leftNeighbour;
        int upNeighbour;
        int downNeighbour;
        int rightNeighbour;
        MPI_Comm cartComm;
        void updateOffset();
        int calcOffset(int N, int p, int rank);
        int minChunkSize(int N, int p);
        int rem(int N, int p);
        void init();
    };
    void debug(int rank, const char *format, ...);
    double get_timer();
    void PrintColMatrix(int m, int n, double *H);
    void PrintRowMatrix(int m, int n, double *H);

}