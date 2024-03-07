#pragma once
#define IDX(I, J) ((J) * Nx + (I))
bool compareSolution(const double *numerical, const double *analytical, int size, double tol );