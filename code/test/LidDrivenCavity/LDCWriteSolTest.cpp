/**
 * @file LDCWriteSolTest.cpp
 * @author Paul Kallarackel (paulkallarackel@gmail.com)
 * @brief Contains unit test for write solution on LDC class
 * @version 0.1
 * @date 2024-03-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "../../include/LidDrivenCavity.h"
#include "../../include/MyTest.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <sstream>
#include <regex>
#include <vector>
#include <cmath>
#include <fstream>
#include <iterator>


BOOST_AUTO_TEST_CASE(LidDrivenCavity_WRITESOLUTION_Test)
{
    /// sync up ranks for test
    MPI_Barrier(MPI_COMM_WORLD);
    /// initalis ecurrent rank, rank size, and cart grid
    int world_size, world_rank;
    MPI_Comm cartComm;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_rank == 0)
        std::cout << "\nLidDrivenCavity WriteSolution Test:" << std::endl;
    int world_p = (int)sqrt(world_size);
    const int dims = 2;
    int sizes[dims] = {world_p, world_p};
    int periods[dims] = {0, 0};
    int reorder = 0;
    MPI_Cart_create(MPI_COMM_WORLD, dims, sizes, periods, reorder, &cartComm);
    /// initlaise settings
    double Lx = 1.0;
    double Ly = 1.0;
    double Nx = 9;
    double Ny = 9;
    double dt = 0.01;
    double T = 1.0;
    double Re = 10;
    /// new LDC instance
    LidDrivenCavity *ldc = new LidDrivenCavity();
    ldc->SetDomainSize(Lx, Ly);
    ldc->SetGridSize(Nx, Ny);
    ldc->SetTimeStep(dt);
    ldc->SetFinalTime(T);
    ldc->SetReynoldsNumber(Re);
    /// Initialise with zero IC (except U velocity on top wall) and write out to text
    ldc->Initialise(world_p, world_rank, cartComm);
    std::string filePath = "output/ic.txt";
    ldc->WriteSolution(filePath);
    /// Only need to check file data on one rank for simplicity
    if (world_rank == 0)
    {
        /// Exoected values for test conditions: max number of empty lines, max number of values in a line, max number of data points
        double *input = new double[3]{1, 6, Nx * Ny};
        double *output = new double[3];

        /// Open the file
        std::ifstream inFile(filePath);
        BOOST_REQUIRE(inFile.is_open()); /// Ensure the file is open

        std::string line;
        /// init counters
        int linesWithValueCount = 0;
        int emptyLineCount = 0;
        int consecutiveEmptyLineCount = 0;
        int maxNumberValues = 0;
        int maxConsecutiveEmptyLineCount = 0;

        /// reading in each line until eof
        while (std::getline(inFile, line))
        {
            if (line.empty())
            {
                consecutiveEmptyLineCount++;
                maxConsecutiveEmptyLineCount = max(consecutiveEmptyLineCount, maxConsecutiveEmptyLineCount);
                emptyLineCount++;
            }
            else
            {
                /// Using stream iterator to extarct number of values
                consecutiveEmptyLineCount = 0;
                std::istringstream iss(line);
                std::vector<double> values((std::istream_iterator<double>(iss)),
                                           std::istream_iterator<double>());
                maxNumberValues = max((int)values.size(), maxNumberValues);
                linesWithValueCount++;
            }
        }

        output[0] = maxConsecutiveEmptyLineCount;
        output[1] = maxNumberValues;
        output[2] = linesWithValueCount;

        BOOST_CHECK(myTest::compareSolution(input, output, 3, 1e-3));

        /// free up memory
        delete[] input;
        delete[] output;
    }
    delete ldc;
    MPI_Comm_free(&cartComm);
    MPI_Barrier(MPI_COMM_WORLD);
}

