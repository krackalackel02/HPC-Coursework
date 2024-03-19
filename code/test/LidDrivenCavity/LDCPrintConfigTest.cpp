/**
 * @file LDCPrintConfigTest.cpp
 * @author Paul Kallarackel (paulkallarackel@gmail.com)
 * @brief This file contains unit-test for print config fucntion on LDC class
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
#include <cmath>
#include <fstream>
#include <iterator>


BOOST_AUTO_TEST_CASE(LidDrivenCavity_PRINTCONFIG_Test)
{
    MPI_Barrier(MPI_COMM_WORLD); /// sync up test
    /// determine local rank and global size
    int world_size, world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    /// only need to check test on rank 0 as will be printed once
    if (world_rank == 0){

        std::cout << "\nLidDrivenCavity PrintConfig Test:" << std::endl;
        // Expected values for terminal print
        double Lx = 1.0;
        double Ly = 1.0;
        double Nx = 9;
        double Ny = 9;
        double N = Nx * Ny;
        double dt = 0.01;
        double T = 1.0;
        double Re = 10;
        double Dx = Lx / (Nx - 1);
        double Dy = Ly / (Ny - 1);
        double steps = ceil(T / dt);

        /// storing expect val in vector and init computed vector
        double *input = new double[11]{Nx, Ny, Dx, Dy, Lx, Ly, N, dt, steps, Re};
        double *readOutput = new double[11]();

        /// Create and configure an instance of LidDrivenCavity
        LidDrivenCavity *ldc = new LidDrivenCavity();
        ldc->SetDomainSize(Lx, Ly);
        ldc->SetGridSize(Nx, Ny);
        ldc->SetTimeStep(dt);
        ldc->SetFinalTime(T);
        ldc->SetReynoldsNumber(Re);

        /// Redirect std::cout to a stringstream
        std::stringstream buffer;
        std::streambuf *prevCoutBuf = std::cout.rdbuf(buffer.rdbuf());

        ldc->PrintConfiguration(); /// Call the method to capture its output

        std::cout.rdbuf(prevCoutBuf); /// Restore the original buffer

        std::string output = buffer.str();
        // std::cout << "Output:\n" << output; // print captured output

        /// Regex to match a floating point number or integer
        std::regex numberPattern("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");

        /// Use regex iterator to find all numbers in the output string and store in readOutput
        std::sregex_iterator iter(output.begin(), output.end(), numberPattern);
        std::sregex_iterator end;
        int count = 0;
        while (iter != end)
        {
            std::smatch match = *iter;
            readOutput[count] = std::stod(match.str());
            count++;
            iter++;
        }
        /// check 2-norm error between expected and printed
        BOOST_CHECK(myTest::compareSolution(input, readOutput, 11, 1e-3));
        /// deallocate memory
        delete ldc;
        delete[] input;
        delete[] readOutput;
    }
    /// sync up ranks for future tests
    MPI_Barrier(MPI_COMM_WORLD);
}
