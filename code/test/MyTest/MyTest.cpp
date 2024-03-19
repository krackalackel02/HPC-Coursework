/**
 * @file MyTest.cpp
 * @author Paul Kallarackel (paulkallarackel@gmail.com)
 * @brief Contains impementation of methods and test fixture in MyTest.h
 * @version 0.1
 * @date 2024-03-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "../../include/MyTest.h"
#include <cblas.h>
#include <iostream>
#include <mpi.h>
#include <omp.h>
#include <cmath>
#include <boost/test/unit_test.hpp>

bool myTest::compareSolution(const double *numerical, const double *analytical, int size, double tol)
{
    /// Uses CBLAS to calc 2norm error in vectors passed
    double *ERR = new double[size];
    cblas_dcopy(size, analytical, 1, ERR, 1);
    cblas_daxpy(size, -1.0, numerical, 1, ERR, 1);
    double e = cblas_dnrm2(size, ERR, 1);
    /// print error and if test pass
    std::cout << "ERROR: " << e << std::endl;
    if (e < tol)
        std::cout << "Test Pass!" << std::endl;
    delete[] ERR;
    return e < tol;
}

bool myTest::compare(double e, double tol)
{
    /// print error and if test pass
    std::cout << "ERROR: " << e << std::endl;
    if (e < tol)
        std::cout << "Test Pass!" << std::endl;
    return e < tol;
}

MPIFixture::MPIFixture()
{
    /// Collects terminal arguements into main file
    argc = boost::unit_test::framework::master_test_suite().argc;
    argv = boost::unit_test::framework::master_test_suite().argv;

    /// Initialising MPI
    int err = MPI_Init(&argc, &argv);
    if (err != MPI_SUCCESS)
    {
        msg = "ERROR initialising MPI";
        BOOST_FAIL(msg);
    }

    /// Initialising Thread Number form env with serial fallback
    int ThreadNum,ThreadID;
    int DEFAULT_ThreadNum = 1;
    char *env_var = getenv("OMP_NUM_THREADS");
    if (env_var != NULL) {
        ThreadNum = atoi(env_var); /// Convert string to integer
    } else {
        ThreadNum = DEFAULT_ThreadNum;
        printf("OMP_NUM_THREADS is not set. Using default number of threads.\nSet env variable with \"export OMP_NUM_THREADS=4\"\n");
    }
    omp_set_num_threads(std::min(ThreadNum,omp_get_max_threads()));

    /// Get the rank and comm size on each process.
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
    if (world_rank == 0) std::cout << "Number of ranks: " << world_size << std::endl;
    #pragma omp parallel private(ThreadID)
	{
		// Obtain and print thread id
		ThreadID = omp_get_thread_num();
		// Only master thread does this
		if (ThreadID == 0&&world_rank==0)
		{
			ThreadNum = omp_get_num_threads();
			std::cout << "Number of threads = " << ThreadNum << std::endl;
		}
	}
    // Sync up ranks before testing
    MPI_Barrier(MPI_COMM_WORLD);

}

MPIFixture::~MPIFixture()
{
    // Close out MPI once all unit tests complete
    MPI_Finalize();
}