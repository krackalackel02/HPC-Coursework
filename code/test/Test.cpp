#include "../include/Test.h"
#include <cblas.h>
#include <iostream>
#include <mpi.h>

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
bool compare(double e, double tol)
{
    std::cout << "ERROR: " << e << std::endl;
    if (e < tol)
        std::cout << "Test Pass!" << std::endl;
    return e < tol;
}
MPIFixture::MPIFixture()
{
    argc = boost::unit_test::framework::master_test_suite().argc;
    argv = boost::unit_test::framework::master_test_suite().argv;
    // std::cout << "Initialising MPI" << std::endl;
    int err = MPI_Init(&argc, &argv);
    if (err != MPI_SUCCESS)
    {
        msg = "ERROR initialising MPI";
        BOOST_FAIL(msg);
    }

    // Get the rank and comm size on each process.
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 1)
    {

        msg = "ERROR use at least 1 process";
        BOOST_FAIL(msg);
    }
    if (sqrt(world_size) != floor(sqrt(world_size)))
    {

        msg = "ERROR use square number process";
        BOOST_FAIL(msg);
    }
    if (err != 0)
    {
        MPI_Finalize(); // Ensure MPI is finalized if there's an error
        BOOST_FAIL(msg);
    }
    if (world_rank == 0) std::cout << "P: " << floor(sqrt(world_size)) << std::endl;

}

MPIFixture::~MPIFixture()
{
    // std::cout << "Finalising MPI" << std::endl;
    MPI_Finalize();
}

BOOST_GLOBAL_FIXTURE(MPIFixture);
