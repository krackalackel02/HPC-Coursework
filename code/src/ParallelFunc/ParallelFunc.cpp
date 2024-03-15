#include "../../include/ParallelFunc.h"
#include <iomanip>

#include <mpi.h>
// Timer library for debug
#include <chrono>
// Library for sleep()
#include <unistd.h>
#include <string>
#include <unordered_map>

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

void prl::gridData::exchangeGhost(double *data,const std::unordered_map<std::string, std::string>& options)
{
    std::string* msgPtr = nullptr;

    // Check if the "msg" option exists in the options map
    auto msgIter = options.find("msg");
    if (msgIter != options.end()) {
        // If "msg" option exists, assign its value to msgPtr
        msgPtr = const_cast<std::string*>(&msgIter->second);
    }
    int nbrs = (int)(upNeighbour >= 0)+(int)(downNeighbour >= 0)+(int)(leftNeighbour >= 0)+(int)(rightNeighbour >= 0);
    MPI_Status recStat[nbrs];
    MPI_Request recReq[nbrs];
    MPI_Request sendReq[nbrs];
    int count = 0;


    if (upNeighbour >= 0){
        MPI_Isend(&data[EXCHGIDX(1, 1)], 1, x_edge_type, upNeighbour, 1, cartComm,&sendReq[count]);
        if(msgPtr) prl::debug(world_rank,"EXCHANGE: Type %s ---- Process %2d I-SENDING UP to %2d\n",msgPtr->c_str(),world_rank, upNeighbour );
        MPI_Irecv(&data[EXCHGIDX(1, 0)], 1, x_edge_type, upNeighbour, 2, cartComm,&recReq[count]);
        if(msgPtr) prl::debug(world_rank,"EXCHANGE: Type %s ---- Process %2d I-RECEIVING UP from %2d\n",msgPtr->c_str(),world_rank, upNeighbour );
        count++;
    }
    if (downNeighbour >= 0){
        MPI_Irecv(&data[EXCHGIDX(1, Chunky+1)], 1, x_edge_type, downNeighbour, 1, cartComm,&recReq[count]);
        if(msgPtr) prl::debug(world_rank,"EXCHANGE: Type %s ---- Process %2d I-SENDING DOWN to %2d\n",msgPtr->c_str(),world_rank, downNeighbour );

        MPI_Isend(&data[EXCHGIDX(1, Chunky)], 1, x_edge_type, downNeighbour, 2, cartComm,&sendReq[count]);
        if(msgPtr) prl::debug(world_rank,"EXCHANGE: Type %s ---- Process %2d I-RECEIVING DOWN from %2d\n",msgPtr->c_str(),world_rank, downNeighbour );

        count++;
    }
    if (leftNeighbour >= 0){
        MPI_Isend(&data[EXCHGIDX(1, 1)], 1, y_edge_type, leftNeighbour, 3, cartComm,&sendReq[count]);
        if(msgPtr) prl::debug(world_rank,"EXCHANGE: Type %s ---- Process %2d I-SENDING LEFT to %2d\n",msgPtr->c_str(),world_rank, leftNeighbour );
        MPI_Irecv(&data[EXCHGIDX(0, 1)], 1, y_edge_type, leftNeighbour, 4, cartComm,&recReq[count]);
        if(msgPtr) prl::debug(world_rank,"EXCHANGE: Type %s ---- Process %2d I-RECEIVING LEFT from %2d\n",msgPtr->c_str(),world_rank, leftNeighbour );
        count++;
    }
    if (rightNeighbour >= 0){
        MPI_Irecv(&data[EXCHGIDX(Chunkx + 1, 1)], 1, y_edge_type, rightNeighbour, 3, cartComm,&recReq[count]);
        if(msgPtr) prl::debug(world_rank,"EXCHANGE: Type %s ---- Process %2d I-SENDING RIGHT to %2d\n",msgPtr->c_str(),world_rank, rightNeighbour );
        MPI_Isend(&data[EXCHGIDX(Chunkx, 1)], 1, y_edge_type, rightNeighbour, 4, cartComm,&sendReq[count]);
        if(msgPtr) prl::debug(world_rank,"EXCHANGE: Type %s ---- Process %2d I-RECEIVING RIGHT from %2d\n",msgPtr->c_str(),world_rank, rightNeighbour );
        count++;
    }
    MPI_Waitall(count,recReq,recStat);
    
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