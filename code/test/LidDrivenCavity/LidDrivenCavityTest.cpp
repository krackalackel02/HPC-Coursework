#include "../../include/LidDrivenCavity.h"
#include "../../include/Test.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <sstream>
#include <regex>
#include <vector>
#include <cmath>
#include <fstream>
#include <iterator>

BOOST_AUTO_TEST_SUITE(LidDrivenCavityTestSuite)

BOOST_AUTO_TEST_CASE(LidDrivenCavity_PRINTCONFIG_Test)
{
    int world_size, world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_rank == 0){

        std::cout << "\nLidDrivenCavity PrintConfig Test:" << std::endl;
        // Original settings
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

        double *input = new double[11]{Nx, Ny, Dx, Dy, Lx, Ly, N, dt, steps, Re};
        double *readOutput = new double[11]();

        // Expected values vector
        std::vector<double> expectedValues{Nx, Ny, Dx, Dy, Lx, Ly, N, dt, steps, Re};

        // Create and configure an instance of LidDrivenCavity
        LidDrivenCavity *ldc = new LidDrivenCavity();
        ldc->SetDomainSize(Lx, Ly);
        ldc->SetGridSize(Nx, Ny);
        ldc->SetTimeStep(dt);
        ldc->SetFinalTime(T);
        ldc->SetReynoldsNumber(Re);

        // Redirect std::cout to a stringstream
        std::stringstream buffer;
        std::streambuf *prevCoutBuf = std::cout.rdbuf(buffer.rdbuf());

        ldc->PrintConfiguration(); // Call the method to capture its output

        std::cout.rdbuf(prevCoutBuf); // Restore the original buffer

        std::string output = buffer.str();
        // std::cout << "Output:\n" << output; // print captured output

        // Regex to match a floating point number or integer
        std::regex numberPattern("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");

        // Use regex iterator to find all matches in the output string
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
        BOOST_CHECK(compareSolution(input, readOutput, 11, 1e-3));
        delete ldc;
        delete[] input;
        delete[] readOutput;
    }
}

BOOST_AUTO_TEST_CASE(LidDrivenCavity_WRITESOLUTION_Test)
{
    // Original settings
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
    if(world_rank==0)std::cout << "HERE" << endl;
    MPI_Cart_create(MPI_COMM_WORLD, dims, sizes, periods, reorder, &cartComm);
    double Lx = 1.0;
    double Ly = 1.0;
    double Nx = 9;
    double Ny = 9;
    double dt = 0.01;
    double T = 1.0;
    double Re = 10;
    LidDrivenCavity *ldc = new LidDrivenCavity();
    ldc->SetDomainSize(Lx, Ly);
    ldc->SetGridSize(Nx, Ny);
    ldc->SetTimeStep(dt);
    ldc->SetFinalTime(T);
    ldc->SetReynoldsNumber(Re);
    ldc->MPIInitialise(world_p, world_rank, cartComm);
    prl::gridData *GRID = ldc->getGRID();
    std::string filePath = "output/ic.txt";
    ldc->MPIWriteSolution(filePath);
    if (world_rank == 0)
    {

        double *input = new double[3]{1, 6, Nx * Ny};
        double *output = new double[3];

        // Open the file
        std::ifstream inFile(filePath);
        BOOST_REQUIRE(inFile.is_open()); // Ensure the file is open

        std::string line;
        int linesWithValueCount = 0;
        int emptyLineCount = 0;
        int consecutiveEmptyLineCount = 0;
        int maxNumberValues = 0;
        int maxConsecutiveEmptyLineCount = 0;

        while (std::getline(inFile, line))
        {
            if (line.empty())
            {
                consecutiveEmptyLineCount++;
                // BOOST_CHECK_LE(consecutiveEmptyLineCount, 1); // Ensure no more than 1 consecutive empty line
                maxConsecutiveEmptyLineCount = max(consecutiveEmptyLineCount, maxConsecutiveEmptyLineCount);
                emptyLineCount++;
            }
            else
            {
                // Reset consecutive empty line count upon encountering a line with values
                consecutiveEmptyLineCount = 0;

                std::istringstream iss(line);
                std::vector<double> values((std::istream_iterator<double>(iss)),
                                           std::istream_iterator<double>());

                // Ensure the line has exactly 6 values
                // BOOST_REQUIRE_EQUAL(values.size(), 6);
                maxNumberValues = max((int)values.size(), maxNumberValues);

                // Increment the count of lines with values
                linesWithValueCount++;
            }
        }

        // Check that there are Nx * Ny lines with values
        // BOOST_CHECK_EQUAL(linesWithValueCount, Nx * Ny);
        output[0] = maxConsecutiveEmptyLineCount;
        output[1] = maxNumberValues;
        output[2] = linesWithValueCount;

        BOOST_CHECK(compareSolution(input, output, 3, 1e-3));

        delete[] input;
        delete[] output;
    }
    delete ldc;
    MPI_Comm_free(&cartComm);
}

BOOST_AUTO_TEST_SUITE_END()