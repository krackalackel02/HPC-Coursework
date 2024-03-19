#include "../include/MyTest.h"

// Main Test module to combine unit tests in
#define BOOST_TEST_MODULE Solver_and_Cavity_Test_Suite
#include <boost/test/included/unit_test.hpp>

// Apply predefined fixture for all tests and init MPI and OMP
BOOST_GLOBAL_FIXTURE(MPIFixture);
