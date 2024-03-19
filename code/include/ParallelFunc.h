/**
 * @file ParallelFunc.h
 * @author Paul Kallarackel (paulkallarackel@gmail.com)
 * @brief Function details main MPI Conceptual grid implemntation
 * Contains logic to distribute Nx*Ny grid amongst ranks and pass on global position data like coords and neighbours
 * @version 0.1
 * @date 2024-03-20
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <mpi.h>
#include <string>
#include <unistd.h>

#define EXCHGIDX(I, J) ((J) * (Chunkx + 2) + (I)) ///< Defines Raw access logic of local vector including pading
#define LOCIDX(I, J) EXCHGIDX(I + 1, J + 1) ///< allows for excluding padding in calcs
/**
 * @brief Holds logic to distribute Nx*Ny grid and rank-to-rank communication
 *
 */
namespace prl
{

    /**
     * @brief Local and Global data object with rank-to-rank comms
     *
     */
    struct gridData
    {

    public:
        int offsetX; ///< variable used to translate local vector coords to global grid position in x
        int offsetY; ///< variable used to translate local vector coords to global grid position in y
        MPI_Datatype x_edge_type; ///< Custom MPI datatype to send edge row data to other ranks
        MPI_Datatype y_edge_type; ///< Custom MPI datatype to send edge col data to other ranks

        /**
         * @brief Custom construct a new grid Data object
         *
         * @param init_Nx Number of x points in LidDrivenCavity object
         * @param init_Ny Number of y points in LidDrivenCavity object
         * @param init_world_p Number of ranks per dimension in LidDrivenCavity object
         * @param init_world_rank Current rank number  in LidDrivenCavity object
         * @param init_cartComm Current Cartesian MPI communicator  in LidDrivenCavity object
         */
        gridData(int init_Nx, int init_Ny, int init_world_p, int init_world_rank, MPI_Comm &init_cartComm);

        /**
         * @brief Default construct a new grid Data object
         *
         */
        gridData();

        /**
         * @brief Destroy the grid Data object
         *
         */
        ~gridData();

        /**
         * @defgroup getCoord Get Cartesian Coordinates
         * @param[in,out] COORD Pointer to an array to store the Cartesian coordinates.
         * @return 
         * @{
         * 
         */
        /**
         * @note coords will be in domain [0,p)
         * @brief Get the Cartesian coordinates of rank.
         */
        void getCartCoord(int *COORD);
        /**
         * @note Coords will be in global Nx*Ny domain
         * @brief Get the top-left coord of the local vector
         */
        void getStart(int *COORD);
        /**
         * @note Coords will be in global Nx*Ny domain
         * @brief Get the bottom-right coord of the local vector
         */
        void getStop(int *COORD);
        /**@}*/


        /**
         * @defgroup getChunkSize Get Chunk Size
         * @note chunk is the size of local vector once global LDC cavity has been diveded into ranks
         * @return length of vector in given direction.
         * @{
         * 
         */
        int getChunkx(); ///< Get the size of the chunk in the x-direction.
        int getChunky(); ///< Get the size of the chunk in the y-direction.
        /**@}*/

        /**
         * @defgroup getCartNeighbour Get Cartesian Neighbours
         * @return Rank of the rank of process within cart comm.
         * @note If no neighbour available, returns -2
         * @{
         * 
         */
        int getLeft(); ///< Get left neighbour
        int getRight(); ///< Get right neighbour
        int getUp(); ///< Get up neighbour
        int getDown(); ///< Get down neighbour
        /// @note will be same as world rank in most cases but implemented in case MPI distributes differently
        int getCenter(); ///< Get current rank within cartesian comm
        /**@}*/


        /**
         * @brief Get the rank of the current process in the cart communicator.
         *
         * @return Rank of the current process.
         */
        int getRank();

        /**
         * @defgroup convLocGlo Convert local <--> global coords
         * @note converts vectors from local [Chunkx,Chunky] domain to and from global [Nx,Ny] domain
         * @param[in,out] locCoord Pointer to local coordiante array
         * @param[in,out] gloCoord Pointer to local coordiante array
         * @return
         * @{
         * 
         */
        void loc2glo(int *locCoord, int *gloCoord); ///< converts local vector coords into global
        void glo2loc(int *locCoord, int *gloCoord); ///< Converts global vector coords into local
         /**@}*/

        /**
         * @brief Main function used to eaxchange any local vector with neighbour ranks and receive into padding
         * Uses Non blocking MPI_Isend and MPI_Irecv to send to neighbour ranks
         * @param[in,out] data Any choice of local vector such as LidDrivenCavity::v
         * @warning cartesian grid must have been initliased first, with new LidDrivenCavity instance and calling LidDrivenCavity::Initialise()
         */
        void exchangeGhost(double *data);

        /**
         * @brief Zeros out padding (edge data) for use in 2-norm error calcs in testing
         * Goes around the padding of the vectors and zeros 
         * @param[in,out] data
         */
        void edgeZero(double *data);

    private:
        int Nx;             ///< Number of global x points
        int Ny;             ///< Number of global y points
        int remx;           ///< Remainder of global x length of grid by number of ranks per dimension
        int remy;           ///< Remainder of global y length of grid by number of ranks per dimension
        int Chunkx;         ///< Local size of vector in x dimension
        int Chunky;         ///< Local size of vector in y dimension
        int world_rank;     ///< Current global Rank number
        int cartRank;       ///< Current Rank number in created grid
        int world_p;        ///< Number of ranks per dimension
        int world_size;     ///< Total number of ranks
        int *cartCoord;     ///< Coordinates of current rank in cartesian grid
        int *start;         ///< Top left corner coordiantes of local vector in global form
        int *stop;          ///< Bottom right corner coordiantes of local vector in global form
        int leftNeighbour;  ///< Rank to the left
        int upNeighbour;    ///< Rank above
        int downNeighbour;  ///< Rank below
        int rightNeighbour; ///< Rank to the right
        MPI_Comm cartComm;  ///< Cartesian Communicator for rank grid

        /**
         * @brief updates offset variable for local->global coordiantes
         *
         */
        void updateOffset();

        /**
         * @brief Calculates offset variable for local->global coordiantes
         * @param N Number of points in dimension
         * @param p Number of ranks in dimension
         * @param rank Current position of rank along dimension
         * @return

         *
        */
        int calcOffset(int N, int p, int rank);

        /**
         * @brief Finds min chunk size for ranks along dimension
         *
         * @param N Number of points in dimension
         * @param p Number of ranks in dimension
         * @return int
         */
        int minChunkSize(int N, int p);

        /**
         * @brief returns remainder of points divided by ranks in dimension
         *
         * @param N Number of points in dimension
         * @param p Number of ranks in dimension
         * @return int
         */
        int rem(int N, int p);

        /**
         * @brief Calculates and applies grid data to object
         *
         */
        void init();
    };

    /**
     * @brief Debugging statemnt when working with mpi that gives rank timestamp as improvement over normal print
     *
     * @param rank Current Rank number
     * @param format String used to print
     * @param ... variable arguements like Printf()
     * @note Will print timstamp|rank|print statement
     * @note use this for debugging any mpi code 
     */
    void debug(int rank, const char *format, ...);

    /**
     * @brief Get Current time
     *
     * @return double
     * @note creates a timer object used for debuggin mpi at beginning of program
     */
    double get_timer();

    /**
     * @brief Prints out matrix stored in row major
     *
     * @param m Number of rows
     * @param n Number of cols
     * @param H Matrix pointer
     * @warning matrix must be stored in row major for print statemnt to work
     */
    void PrintRowMatrix(int m, int n, double *H);

    /**
     * @brief Copy of sleep function for debugging with MPI
     *
     * @param n Number of seconds to wait
     */
    void time(int n);
}