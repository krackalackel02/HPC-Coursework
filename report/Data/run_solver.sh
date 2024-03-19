#!/bin/bash
# Define parameters
Nt=1
export OMP_NUM_THREADS=$Nt
Lx=1
Ly=1
Nx=401
Ny=401
Re=1000
dt=0.001
T=1
# Write problem settings to a file
echo "Problem settings:" > timings.txt
echo "Lx=$Lx Ly=$Ly Nx=$Nx Ny=$Ny Re=$Re dt=$dt T=$T" >> timings.txt
echo "MPIRANKS [#]  |  OMPTHREADS [#] |  Initialising [s] |  WriteIC [s]  |  Integrate [s]  |  WriteSOL [s]  |  Total [s]" >> timings.txt
echo "--------------------------------------------"
# Loop through square numbers from 1 to 4
for (( i=1; i<=4; i=i+1 ))
do
        Np=$(( $i*$i ))
        echo "Running with $Np MPI ranks"
        echo "Running with $Nt OMP threads"
        # Run the program
        mpirun --bind-to none -np $Np ../../bin/solver.out --Lx $Lx --Ly $Ly --Nx $Nx --Ny $Ny --Re $Re --dt $dt --T $T
        echo "--------------------------------------------"  
done
# Reset Np
Np=1
# Loop through OMP threads from 1 to 16
Nt=1
# echo "Running with $Np MPI ranks"
# echo "Running with $Nt OMP threads"
# export OMP_NUM_THREADS=$Nt
# # Run the program
# mpirun --bind-to none -np $Np ../../bin/solver.out --Lx $Lx --Ly $Ly --Nx $Nx --Ny $Ny --Re $Re --dt $dt --T $T
echo "--------------------------------------------"
for (( i=1; i<=16; i=i+1 ))
do
    Nt=$(( $i*1 ))
    echo "Running with $Np MPI ranks"
    echo "Running with $Nt OMP threads"
    export OMP_NUM_THREADS=$Nt
    # Run the program
    mpirun --bind-to none -np $Np ../../bin/solver.out --Lx $Lx --Ly $Ly --Nx $Nx --Ny $Ny --Re $Re --dt $dt --T $T
    echo "--------------------------------------------"  
done