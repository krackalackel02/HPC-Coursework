/**
 * @file SolverCG.h
 * @author Paul Kallarackel (paulkallarackel@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-03-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
/**
 * @class SolverCG
 * @brief This class sets up the solver using the conjugate gradient method
 * 
 * This class gets utilised by the problem defined by the class found in "LidDrivenCavity.h".
 * It implements a finite difference scheme and then uses the Solve method which is based off the conjugate gradient method found in AERO70011 notes.
 * It also imposes boundary conditions to ensure an accurate solution.
 */
class SolverCG
{
public:
    /**
     * @brief Construct a new Solver C G object
     * 
     * @param pNx Number of x points
     * @param pNy Number of y points
     * @param pdx X step size
     * @param pdy Y step size
     */
    SolverCG(int pNx, int pNy, double pdx, double pdy);
    /**
     * @brief Destroy the Solver C G object
     * 
     */
    ~SolverCG();
    /**
     * @brief Method used to solve for X in problem, where A and B are known: A*X = B
     * 
     * @param b Vector Result B
     * @param x Vector State X0, initial guess for solution on each time step
     */
    void Solve(double* b, double* x);

private:
    double dx; /// X step size
    double dy; /// Y step size
    int Nx; /// Number of x points
    int Ny; /// Number of y points
    double* r; /// Vector representing error in current solution, R: R0 = B-A*X0, Rk+1 = Rk - alphak*Pk
    double* p; /// Vector representing gradient in solution e.g. delta to next guess
    double* z; /// Vector representing the preconditioned R vector used to updated temporary state
    double* t; /// Vector representing temporary state
    /**
     * @brief This function applies the operation to an input: A*p = t
     * 
     * @param p Vector representing gradient in solution e.g. delta to next guess
     * @param t Vector representing temporary state
     */
    void ApplyOperator(double* p, double* t);
    /**
     * @brief Calculates vorticity at the "walls" as the conjugate gradient will not solve for "wall"/edge locations
     * 
     * @param p Vector representing gradient in solution e.g. delta to next guess
     * @param t Vector representing temporary state
     */
    void Precondition(double* p, double* t);
    /**
     * @brief Sets initial vorticity at the "walls" to be 0
     * 
     * @param p Vector representing gradient in solution e.g. delta to next guess
     */
    void ImposeBC(double* p);

};

