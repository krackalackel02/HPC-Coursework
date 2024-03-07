#include "../include/Test.h"
#include <cblas.h>
#include <iostream>

#define BOOST_TEST_MODULE Solver_and_Cavity_Test_Suite

// ... BEFORE including the Boost test header
#include <boost/test/included/unit_test.hpp>

// Function to compare numerical and analytical solutions (if available)
bool compareSolution(const double *numerical, const double *analytical, int size, double tol)
{
    double *ERR = new double[size];
    cblas_dcopy(size, analytical, 1, ERR, 1);
    cblas_daxpy(size, -1.0, numerical, 1, ERR, 1);
    double e = cblas_dnrm2(size, ERR, 1);
    std::cout << "ERROR: " << e << std::endl;
    delete[] ERR;
    if (e < tol)
        std::cout << "Test Pass!" << std::endl;
    return e < tol;
}
