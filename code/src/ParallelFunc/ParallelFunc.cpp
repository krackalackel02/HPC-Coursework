#include "../../include/ParallelFunc.h"
#include <iomanip>

#include <mpi.h>
// Timer library for debug
#include <chrono>
// Library for sleep()
#include <unistd.h>

prl::gridData::gridData(int init_Nx, int init_Ny, int init_world_p, int init_world_rank, MPI_Comm init_cartComm)
{
    Nx = init_Nx;
    Ny = init_Ny;
    world_rank = init_world_rank;
    world_p = init_world_p;
    cartComm = init_cartComm;
    init();
}
prl::gridData::gridData()
{
}
void prl::gridData::init()
{
    world_size = world_p * world_p;
    offsetX = 0;
    offsetY = 0;
    cartCoord = new int[2];
    start = new int[2]();
    MPI_Cart_coords(cartComm, world_rank, 2, cartCoord);
    remx = rem(Nx, world_p);
    remy = rem(Ny, world_p);
    int minx = minChunkSize(Nx, world_p);
    Chunkx = cartCoord[1] < remx ? minx + 1 : minx;
    int miny = minChunkSize(Ny, world_p);
    Chunky = cartCoord[0] < remy ? miny + 1 : miny;
    stop = new int[2]{Chunkx - 1, Chunky - 1};
    loc2glo(start, start);
    loc2glo(stop, stop);
    MPI_Cart_rank(cartComm, cartCoord, &cartRank);
    MPI_Cart_shift(cartComm, 0, 1, &upNeighbour, &downNeighbour);
    MPI_Cart_shift(cartComm, 1, 1, &leftNeighbour, &rightNeighbour);

    // X edge
    MPI_Type_vector(Chunkx, 1, 1, MPI_DOUBLE, &x_edge_type);
    MPI_Type_commit(&x_edge_type);

    // Y edge
    MPI_Type_vector(Chunky, 1, (Chunkx + 2), MPI_DOUBLE, &y_edge_type);
    MPI_Type_commit(&y_edge_type);
}
prl::gridData::~gridData()
{
    delete[] cartCoord;
    delete[] start;
    delete[] stop;
    std::cout << "Deleting GRID object at address " << this << std::endl;
}

int prl::gridData::getChunkx()
{
    return Chunkx;
}
int prl::gridData::getChunky()
{
    return Chunky;
}
int prl::gridData::getDown()
{
    return downNeighbour;
}
int prl::gridData::getCenter()
{
    return cartRank;
}
int prl::gridData::getUp()
{
    return upNeighbour;
}
int prl::gridData::getLeft()
{
    return leftNeighbour;
}
int prl::gridData::getRight()
{
    return rightNeighbour;
}
void prl::gridData::getCartCoord(int *COORD)
{
    COORD[0] = cartCoord[0];
    COORD[1] = cartCoord[1];
}
void prl::gridData::getStart(int *COORD)
{
    COORD[0] = start[0];
    COORD[1] = start[1];
}
void prl::gridData::getStop(int *COORD)
{
    COORD[0] = stop[0];
    COORD[1] = stop[1];
}

void prl::gridData::loc2glo(int *locCoord, int *gloCoord)
{
    updateOffset();
    gloCoord[0] = locCoord[0] + offsetX;
    gloCoord[1] = locCoord[1] + offsetY;
}
void prl::gridData::glo2loc(int *locCoord, int *gloCoord)
{
    updateOffset();
    locCoord[0] = gloCoord[0] - offsetX;
    locCoord[1] = gloCoord[1] - offsetY;
}

void prl::gridData::updateOffset()
{
    // X:
    offsetX = calcOffset(Nx, world_p, cartCoord[1]);
    // Y:
    offsetY = calcOffset(Ny, world_p, cartCoord[0]);
}
int prl::gridData::rem(int N, int p)
{
    return N % p;
}
int prl::gridData::calcOffset(int N, int p, int rank)
{
    int r = rem(N, p);
    int min = (N - r) / p;
    if (rank < r)
    {
        return rank * (min + 1);
    }
    else
    {

        return r * (min + 1) + (rank - r) * min;
    }
}
int prl::gridData::minChunkSize(int N, int p)
{
    int r = rem(N, p);
    return (N - r) / p;
}

double prl::get_timer()
{
    using namespace std::chrono;
    static auto start = high_resolution_clock::now();               // Store the start time at first call
    auto now = high_resolution_clock::now();                        // Current time at each call
    auto elapsed = duration_cast<nanoseconds>(now - start).count(); // Elapsed time in nanoseconds
    return static_cast<double>(elapsed) / 1e9;                      // Convert to double and return
}
void prl::debug(int rank, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("%12.6f|%2d|", get_timer(), rank);
    vprintf(format, args);
    va_end(args);
}

void prl::gridData::exchangeGhost(double *data,const char msg)
{

    int tag;
    MPI_Status status;

    // Send to Up  / Receive from Down
    tag = 1+exchangeCall;
    if (upNeighbour >= 0)
    {
        MPI_Send(&data[EXCHGIDX(1, 1)], 1, x_edge_type, upNeighbour, tag, cartComm);
        // prl::debug(world_rank,"Type %2c Process %2d sending UP to %2d\n",msg,world_rank, upNeighbour );
        // if(world_rank==7)prl::debug(world_rank,"Type %2c Process %2d sending UP to %2d\t\t data: %2d  %2d  %2d  %2d\n",msg,world_rank, upNeighbour,data[LOCIDX(1, 1)],data[LOCIDX(2, 1)],data[LOCIDX(3, 1)],data[LOCIDX(4, 1)] );
    }
    if (downNeighbour >= 0)
    {
        MPI_Recv(&data[EXCHGIDX(1, Chunky+1)], 1, x_edge_type, downNeighbour, tag, cartComm, &status);
        // prl::debug(world_rank,"Type %2c Process %2d received DOWN from %2d\n",msg,world_rank, downNeighbour );
        // if(world_rank==4)prl::debug(world_rank,"Type %2c Process %2d received DOWN from %2d\t\t data: %2d  %2d  %2d  %2d\n",msg,world_rank, downNeighbour,data[LOCIDX(1, Chunky+1)],data[LOCIDX(2, Chunky+1)],data[LOCIDX(3, Chunky+1)],data[LOCIDX(4, Chunky+1)] );
    }

    // Send to Down  / Receive from Up
    tag = 2+exchangeCall;

    if (downNeighbour >= 0)
    {
        MPI_Send(&data[EXCHGIDX(1, Chunky)], 1, x_edge_type, downNeighbour, tag, cartComm);
        // prl::debug(world_rank,"Type %2c Process %2d sending DOWN to %2d\n",msg,world_rank, downNeighbour );
        // if(world_rank==1)prl::debug(world_rank,"Type %2c Process %2d sending DOWN to %2d\t\t data: %2d  %2d  %2d  %2d\n",msg,world_rank, downNeighbour,data[LOCIDX(1, Chunky)],data[LOCIDX(2, Chunky)],data[LOCIDX(3, Chunky)],data[LOCIDX(4, Chunky)] );
    }
    if (upNeighbour >= 0)
    {
        MPI_Recv(&data[EXCHGIDX(1, 0)], 1, x_edge_type, upNeighbour, tag, cartComm, &status);
        // prl::debug(world_rank,"Type %2c Process %2d received UP from %2d\n",msg,world_rank, upNeighbour );
        // if(world_rank==4)prl::debug(world_rank,"Type %2c Process %2d received UP from %2d\t\t data: %2d  %2d  %2d  %2d  \n",msg,world_rank, upNeighbour,data[LOCIDX(1, 0)],data[LOCIDX(2, 0)],data[LOCIDX(3, 0)],data[LOCIDX(4, 0)] );
    }

    // Send to Left  / Receive from Right
    tag = 3+exchangeCall;

    if (leftNeighbour >= 0)
    {
        MPI_Send(&data[EXCHGIDX(1, 1)], 1, y_edge_type, leftNeighbour, tag, cartComm);
        // prl::debug(world_rank,"Type %2c Process %2d sending LEFT to %2d\n",msg,world_rank, leftNeighbour );
    }
    if (rightNeighbour >= 0)
    {
        MPI_Recv(&data[EXCHGIDX(Chunkx + 1, 1)], 1, y_edge_type, rightNeighbour, tag, cartComm, &status);
        // prl::debug(world_rank,"Type %2c Process %2d received RIGHT from %2d\n",msg,world_rank, rightNeighbour );
    }

    // Send to Right  / Receive from Left
    tag = 4+exchangeCall;

    if (rightNeighbour >= 0)
    {
        MPI_Send(&data[EXCHGIDX(Chunkx, 1)], 1, y_edge_type, rightNeighbour, tag, cartComm);
        // prl::debug(world_rank,"Type %2c Process %2d sending RIGHT to %2d\n",msg,world_rank, rightNeighbour );
    }
    if (leftNeighbour >= 0)
    {
        MPI_Recv(&data[EXCHGIDX(0, 1)], 1, y_edge_type, leftNeighbour, tag, cartComm, &status);
        // prl::debug(world_rank,"Type %2c Process %2d received LEFT from %2d\n",msg,world_rank, leftNeighbour );
    }
    exchangeCall++;
}
void prl::PrintColMatrix(int m, int n, double *H)
{
    for (int i = 0; i < m; ++i)
    {
        std::cout<< "Row "<<i <<" : ";
        for (int j = 0; j < n; ++j)
        {
            std::cout << std::setw(5) << H[j * m + i] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
void prl::PrintRowMatrix(int m, int n, double *H)
{
    for (int i = 0; i < n; ++i)
    {
        std::cout<< "Row "<<i <<" : ";
        for (int j = 0; j < m; ++j)
        {
            std::cout << std::setw(5) << H[i * m + j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}