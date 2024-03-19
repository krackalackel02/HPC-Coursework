/**
 * @file MyTest.h
 * @author Paul Kallarackel (paulkallarackel@gmail.com)
 * @brief File defines the global fixture to use parallel testing
 * @version 0.1
 * @date 2024-03-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include <string>
#include <mpi.h>

/// Holds some fucntions used for testing checking
namespace myTest
{
    /**
     * @brief Function compares ordered vector against another and computes 2-norm against tolerance
     * 
     * @param numerical Numerical / Computed solution vector
     * @param analytical Analtical / Expected solution vector
     * @param size Size of vectors passed
     * @param tol Tolerance to check against
     * @return true 
     * @return false 
     */
    bool compareSolution(const double *numerical, const double *analytical, int size, double tol);

    /**
     * @brief Compares Error against specificed tolerance having precomputed error
     * 
     * @param e Error
     * @param tol Tolerance
     * @return true 
     * @return false 
     */
    bool compare(double e, double tol);


}

/// Used for instantiation of unit test file with MPI once
struct MPIFixture
{
public:

    /**
     * @brief Construct a new MPIFixture object used to initialise  MPI and Open MP threads for all tests once
     * 
     */
    explicit MPIFixture();

    /**
     * @brief Destroy the MPIFixture object
     * 
     */
    ~MPIFixture();

    int argc; /**< main file arguements*/
    char **argv; /**< main file arguements*/
    int world_p; /**< square root of number of ranks*/
    int world_size; /**< number of ranks*/
    int world_rank; /**< current rank number*/
    std::string msg = ""; /**< err message prop*/
};