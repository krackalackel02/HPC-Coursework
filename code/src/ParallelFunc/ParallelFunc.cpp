#include "../../include/ParallelFunc.h"
#include <mpi.h>

prl::gridData::gridData(int init_Nx, int init_Ny, int init_world_p, int init_world_rank, MPI_Comm cartComm)
    : offsetX(0), offsetY(0), Nx(init_Nx), Ny(init_Ny), world_rank(init_world_rank), world_p(init_world_p), world_size(init_world_p * init_world_p)
{
    cartCoord = new int[2];
    start = new int[2]();
    MPI_Cart_coords(cartComm, world_rank, 2, cartCoord);
    remx = rem(init_Nx, init_world_p);
    remy = rem(init_Ny, init_world_p);
    int minx = minChunkSize(init_Nx, init_world_p);
    Chunkx = cartCoord[1] < remx ? minx + 1 : minx;
    int miny = minChunkSize(init_Ny, init_world_p);
    Chunky = cartCoord[1] < remy ? miny + 1 : miny;
    stop = new int[2]{Chunky - 1, Chunkx - 1};
    loc2glo(start, start);
    loc2glo(stop, stop);
}

prl::gridData::~gridData()
{
    delete[] cartCoord;
    delete[] start;
    delete[] stop;
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
    gloCoord[0] = locCoord[0] + offsetY;
    gloCoord[1] = locCoord[1] + offsetX;
}
void prl::gridData::glo2loc(int *locCoord, int *gloCoord)
{
    updateOffset();
    locCoord[0] = gloCoord[0] - offsetY;
    locCoord[1] = gloCoord[1] - offsetX;
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