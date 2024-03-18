#include "../include/MyTest.h"

// ... BEFORE including the Boost test header
#define BOOST_TEST_MODULE Solver_and_Cavity_Test_Suite
#include <boost/test/included/unit_test.hpp>

BOOST_GLOBAL_FIXTURE(MPIFixture);
