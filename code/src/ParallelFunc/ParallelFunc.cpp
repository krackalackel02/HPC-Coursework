#include "../../include/ParallelFunc.h"
#include <iomanip>
#include <mpi.h>
#include <chrono>
#include <unistd.h>
#include <string>
#include <omp.h>

prl::gridData::gridData(int init_Nx, int init_Ny, int init_world_p, int init_world_rank, MPI_Comm &init_cartComm)
{
    /// Assign global LDC parameters to each ranks GRID object
    Nx = init_Nx;
    Ny = init_Ny;
    world_rank = init_world_rank;
    world_p = init_world_p;
    cartComm = init_cartComm;
    /// Initialise local vectors based on these parameters
    init();
}

prl::gridData::gridData()
{
}

void prl::gridData::init()
{
    world_size = world_p * world_p;
    /// Reset Offset
    offsetX = 0;
    offsetY = 0;

    /// get ranks coordinates within cartesian communicator
    cartCoord = new int[2];
    MPI_Cart_coords(cartComm, world_rank, 2, cartCoord);

    /// calc remainder for chunck size calc
    remx = rem(Nx, world_p);
    remy = rem(Ny, world_p);

    /// Calc chunk size based of ranks coordinates in grid
    int minx = minChunkSize(Nx, world_p);
    Chunkx = cartCoord[1] < remx ? minx + 1 : minx;
    int miny = minChunkSize(Ny, world_p);
    Chunky = cartCoord[0] < remy ? miny + 1 : miny;

    /// find top left and bottom right global coord for ranks local vector
    start = new int[2]();
    stop = new int[2]{Chunkx - 1, Chunky - 1};
    loc2glo(start, start);
    loc2glo(stop, stop);

    /// Finf each neighbour of rank: -2 if no neighbour
    MPI_Cart_rank(cartComm, cartCoord, &cartRank);
    MPI_Cart_shift(cartComm, 0, 1, &upNeighbour, &downNeighbour);
    MPI_Cart_shift(cartComm, 1, 1, &leftNeighbour, &rightNeighbour);

    /// X edge to send
    MPI_Type_vector(Chunkx, 1, 1, MPI_DOUBLE, &x_edge_type);
    MPI_Type_commit(&x_edge_type);

    /// Y edge to send
    MPI_Type_vector(Chunky, 1, (Chunkx + 2), MPI_DOUBLE, &y_edge_type);
    MPI_Type_commit(&y_edge_type);
}

prl::gridData::~gridData()
{
    /// delete dynamic error allocated
    delete[] cartCoord;
    cartCoord = nullptr;
    delete[] start;
    start = nullptr;
    delete[] stop;
    stop = nullptr;
    MPI_Type_free(&x_edge_type);
    MPI_Type_free(&y_edge_type);
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
    /// global form of local coords is offset base dof where rank is in virt top.
    gloCoord[0] = locCoord[0] + offsetX;
    gloCoord[1] = locCoord[1] + offsetY;
}

void prl::gridData::glo2loc(int *locCoord, int *gloCoord)
{
    updateOffset();
    /// global form of local coords is offset base dof where rank is in virt top.
    locCoord[0] = gloCoord[0] - offsetX;
    locCoord[1] = gloCoord[1] - offsetY;
}

void prl::gridData::updateOffset()
{
    /// X: used cartCoord[1] as (y,x) in MPI
    offsetX = calcOffset(Nx, world_p, cartCoord[1]);
    /// Y: used cartCoord[0] as (y,x) in MPI
    offsetY = calcOffset(Ny, world_p, cartCoord[0]);
}

int prl::gridData::rem(int N, int p)
{
    return N % p;
}

int prl::gridData::calcOffset(int N, int p, int rank)
{
    /// Finds offset based of whether ``big'' or ``small'' Chunk
    int r = rem(N, p);
    int min = (N - r) / p;
    /// Big
    if (rank < r)
    {
        return rank * (min + 1);
    }
    /// small
    else
    {

        return r * (min + 1) + (rank - r) * min;
    }
}

int prl::gridData::minChunkSize(int N, int p)
{
    /// Min chunk size from load balncing algo
    int r = rem(N, p);
    return (N - r) / p;
}

double prl::get_timer()
{
    using namespace std::chrono;
    static auto start = high_resolution_clock::now();               /// Store the start time at first call
    auto now = high_resolution_clock::now();                        /// Current time at each call
    auto elapsed = duration_cast<nanoseconds>(now - start).count(); /// Elapsed time in nanoseconds
    return static_cast<double>(elapsed) / 1e9;                      /// Convert to double and return
}

void prl::debug(int rank, const char *format, ...)
{
    /// Uses variabel args to maintain printf functionality with timestamp and rnak number
    va_list args;
    va_start(args, format);
    printf("%12.6f|%2d|", get_timer(), rank);
    vprintf(format, args);
    va_end(args);
}

void prl::gridData::exchangeGhost(double *data)
{
    /// Number of rank neighbours
    int nbrs = (int)(upNeighbour >= 0) + (int)(downNeighbour >= 0) + (int)(leftNeighbour >= 0) + (int)(rightNeighbour >= 0);

    /// Number of status and requests
    MPI_Status recStat[nbrs];
    MPI_Request recReq[nbrs];
    MPI_Request sendReq[nbrs];

    /// If Neighbour found iterate count such that request array indexed properly as message sent
    int count = 0;

    /// Checks if up neighbour
    if (upNeighbour >= 0)
    {
        /// Non blocking REceive into padding
        MPI_Isend(&data[EXCHGIDX(1, 1)], 1, x_edge_type, upNeighbour, 1, cartComm, &sendReq[count]);
        /// If terminal print requested
        /// Non blocking REceive into padding
        MPI_Irecv(&data[EXCHGIDX(1, 0)], 1, x_edge_type, upNeighbour, 2, cartComm, &recReq[count]);
        count++;
    }
    if (downNeighbour >= 0)
    {
        /// Non blocking REceive into padding
        MPI_Irecv(&data[EXCHGIDX(1, Chunky + 1)], 1, x_edge_type, downNeighbour, 1, cartComm, &recReq[count]);

        /// Non blocking REceive into padding
        MPI_Isend(&data[EXCHGIDX(1, Chunky)], 1, x_edge_type, downNeighbour, 2, cartComm, &sendReq[count]);

        count++;
    }
    if (leftNeighbour >= 0)
    {
        /// Non blocking REceive into padding
        MPI_Isend(&data[EXCHGIDX(1, 1)], 1, y_edge_type, leftNeighbour, 3, cartComm, &sendReq[count]);
        /// Non blocking REceive into padding
        MPI_Irecv(&data[EXCHGIDX(0, 1)], 1, y_edge_type, leftNeighbour, 4, cartComm, &recReq[count]);
        count++;
    }
    if (rightNeighbour >= 0)
    {
        /// Non blocking REceive into padding
        MPI_Irecv(&data[EXCHGIDX(Chunkx + 1, 1)], 1, y_edge_type, rightNeighbour, 3, cartComm, &recReq[count]);
        /// Non blocking REceive into padding
        MPI_Isend(&data[EXCHGIDX(Chunkx, 1)], 1, y_edge_type, rightNeighbour, 4, cartComm, &sendReq[count]);
        count++;
    }
    // Free the MPI requests
    if (count > 0)
    {
        for (int i = 0; i < count; ++i)
        {
            if(sendReq[i]!=MPI_REQUEST_NULL)MPI_Request_free(&sendReq[i]);
        }
    }
    /// Wait on Receives as data needed for next operation
    MPI_Waitall(count, recReq, recStat);
}

void prl::gridData::edgeZero(double *data)
{
/// Go around edges of vector and zero out
#pragma omp parallel for schedule(static)
    for (int i = 0; i < Chunkx + 2; ++i)
    {
        data[EXCHGIDX(i, 0)] = 0.0;
    }
#pragma omp parallel for schedule(static)
    for (int i = 0; i < Chunkx + 2; ++i)
    {
        data[EXCHGIDX(i, Chunky + 1)] = 0.0;
    }
#pragma omp parallel for schedule(static)
    for (int j = 0; j < Chunky + 2; ++j)
    {
        data[EXCHGIDX(0, j)] = 0.0;
    }
#pragma omp parallel for schedule(static)
    for (int j = 0; j < Chunky + 2; ++j)
    {
        data[EXCHGIDX(Chunkx + 1, j)] = 0.0;
    }
}

void prl::PrintRowMatrix(int m, int n, double *H)
{
    for (int i = 0; i < n; ++i)
    {
        std::cout << "Row " << i << " : ";
        for (int j = 0; j < m; ++j)
        {
            std::cout << std::setw(5) << H[i * m + j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void prl::time(int n)
{
    sleep(n);
}
