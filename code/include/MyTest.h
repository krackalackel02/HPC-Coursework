#pragma once
#ifndef TEST_H
#define TEST_H

#define IDX(I, J) ((J) * Nx + (I))

#include <string>
#include <mpi.h>

bool compareSolution(const double *numerical, const double *analytical, int size, double tol);
bool compare( double e, double tol);

struct MPIFixture {
public:
    explicit MPIFixture();

    ~MPIFixture();

    int argc;
    char **argv;
    int world_p;
    int world_size;
    int world_rank;
    std::string msg = "";
    MPI_Comm cartComm;
};

#endif // TEST_H
