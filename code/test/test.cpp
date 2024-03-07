#include "../include/LidDrivenCavity.h"
#include "../include/SolverCG.h"
#include <cmath>
#include <cblas.h>
#include <iostream>
#include <sstream>
#include <regex>
#include <vector>

#define IDX(I, J) ((J) * Nx + (I))

#define BOOST_TEST_MODULE Solver_and_Cavity_Test_Suite

// ... BEFORE including the Boost test header
#include <boost/test/included/unit_test.hpp>

// Function to compare numerical and analytical solutions (if available)
bool compareSolution(const double *numerical, const double *analytical, int size, double tol = 1e-3)
{
    double *ERR = new double[size];
    cblas_dcopy(size, analytical, 1, ERR, 1);
    cblas_daxpy(size, -1.0, numerical, 1, ERR, 1);
    double e = cblas_dnrm2(size, ERR, 1);
    cout << "ERROR: " << e << endl;
    delete[] ERR;
    if(e<tol)cout << "Test Pass!" << endl;
    return e < tol;
}

BOOST_AUTO_TEST_CASE(SolverCG_SOLVE_AnalayticalTest)
{
    int Nx = 2e3; // Example grid size
    int Ny = 2e3; // Example grid size
    double Lx = 1.0; // Example grid size
    double Ly = 1.0; // Example grid size
    double dx = Lx / (double)(Nx - 1);
    double dy = Ly / (double)(Ny - 1);
    int N = Nx * Ny;

    /// Analytical Solution
    cout << "\nSolverCG Analaytical Test:" << endl;
    SolverCG *solver = new SolverCG(Nx, Ny, dx, dy);
    double *solution = new double[N];
    double *analytical = new double[N];
    double *v = new double[N];
    const double k = 2.0 / Lx;
    const double l = 2.0 / Ly;
    for (int i = 0; i < Nx; ++i)
    {
        for (int j = 0; j < Ny; ++j)
        {
            v[IDX(i, j)] = -M_PI * M_PI * (k * k + l * l) * sin(M_PI * k * i * dx) * sin(M_PI * l * j * dy);
        }
    }
    for (int i = 0; i < Nx; ++i)
    {
        for (int j = 0; j < Ny; ++j)
        {
            solution[IDX(i, j)] = -sin(M_PI * k * i * dx) * sin(M_PI * l * j * dy);
        }
    }
    solver->Solve(v, analytical);
    BOOST_CHECK(compareSolution(solution, analytical, N));

    delete solver;
    delete[] solution;
    delete[] analytical;
    delete[] v;
}

BOOST_AUTO_TEST_CASE(SolverCG_SOLVE_ZeroTest){
    /// Zero input solution
    cout << "\nSolverCG Zero Test:" << endl;
    int Nx = 1e2; // Example grid size
    int Ny = 1e2; // Example grid size
    double Lx = 1.0; // Example grid size
    double Ly = 1.0; // Example grid size
    double dx = Lx / (double)(Nx - 1);
    double dy = Ly / (double)(Ny - 1);
    int N = Nx * Ny;
    SolverCG *smallSolver = new SolverCG(Nx, Ny, dx, dy);
    double* smallSolution = new double[N]{};
    double* smallAnalytical = new double[N];
    double* smallV = new double[N]{1e-8};
    smallSolver->Solve(smallV, smallAnalytical);
    BOOST_CHECK(compareSolution(smallSolution, smallAnalytical, N));
    delete smallSolver;
    delete[] smallSolution;
    delete[] smallAnalytical;
    delete[] smallV;
}

BOOST_AUTO_TEST_CASE(LidDrivenCavity_PRINTCONFIG_Test) {
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

    double* input = new double[11]{Nx, Ny, Dx, Dy, Lx, Ly, N, dt, steps, Re};
    double* readOutput = new double[11]();

    // Expected values vector
    std::vector<double> expectedValues{Nx, Ny, Dx, Dy, Lx, Ly, N, dt, steps, Re};

    cout << "\nLidDrivenCavity PrintConfig Test:" << endl;
    // Create and configure an instance of LidDrivenCavity
    LidDrivenCavity* ldc = new LidDrivenCavity();
    ldc->SetDomainSize(Lx, Ly);
    ldc->SetGridSize(Nx, Ny);
    ldc->SetTimeStep(dt);
    ldc->SetFinalTime(T);
    ldc->SetReynoldsNumber(Re);


    // Redirect cout to a stringstream
    std::stringstream buffer;
    std::streambuf* prevCoutBuf = std::cout.rdbuf(buffer.rdbuf());

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
    while (iter != end) {
        std::smatch match = *iter;
        readOutput[count] = std::stod(match.str());
        count++;
        iter++;
    }
    BOOST_CHECK(compareSolution(input,readOutput, 11));
    

    delete ldc;
    delete[] input;
    delete[] readOutput;
}

BOOST_AUTO_TEST_CASE(LidDrivenCavity_WRITESOLUTION_Test) {
    // Original settings
    double Lx = 1.0;
    double Ly = 1.0;
    double Nx = 9;
    double Ny = 9;
    double dt = 0.01;
    double T = 1.0;
    double Re = 10;

    cout << "\nLidDrivenCavity WriteSolution Test:" << endl;

    // Create and configure an instance of LidDrivenCavity
    LidDrivenCavity* ldc = new LidDrivenCavity();
    ldc->SetDomainSize(Lx, Ly);
    ldc->SetGridSize(Nx, Ny);
    ldc->SetTimeStep(dt);
    ldc->SetFinalTime(T);
    ldc->SetReynoldsNumber(Re);
    ldc->Initialise();
    std::string filePath = "output/ic.txt";
    ldc->WriteSolution(filePath);

    double* input = new double[3]{1,6,Nx*Ny};
    double* output = new double[3];
    
    // Open the file
    std::ifstream inFile(filePath);
    BOOST_REQUIRE(inFile.is_open()); // Ensure the file is open

    std::string line;
    int linesWithValueCount = 0;
    int emptyLineCount = 0;
    int consecutiveEmptyLineCount = 0;
    int maxNumberValues  = 0;
    int maxConsecutiveEmptyLineCount  = 0;

    while (std::getline(inFile, line)) {
        if (line.empty()) {
            consecutiveEmptyLineCount++;
            // BOOST_CHECK_LE(consecutiveEmptyLineCount, 1); // Ensure no more than 1 consecutive empty line
            maxConsecutiveEmptyLineCount = max(consecutiveEmptyLineCount,maxConsecutiveEmptyLineCount);
            emptyLineCount++;
        } else {
            // Reset consecutive empty line count upon encountering a line with values
            consecutiveEmptyLineCount = 0;

            std::istringstream iss(line);
            std::vector<double> values((std::istream_iterator<double>(iss)),
                                       std::istream_iterator<double>());

            // Ensure the line has exactly 6 values
            // BOOST_REQUIRE_EQUAL(values.size(), 6);
            maxNumberValues = max((int)values.size(),maxNumberValues);

            // Increment the count of lines with values
            linesWithValueCount++;
        }
    }

    // Check that there are Nx * Ny lines with values
    // BOOST_CHECK_EQUAL(linesWithValueCount, Nx * Ny);
    output[0] = maxConsecutiveEmptyLineCount;
    output[1] = maxNumberValues;
    output[2] = linesWithValueCount;

    BOOST_CHECK(compareSolution(input,output, 3));

    delete ldc;
    delete[] input;
    delete[] output;
}