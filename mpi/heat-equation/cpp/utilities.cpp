// Utility functions for heat equation solver

#include <mpi.h>

#include "heat.hpp"

// Calculate average temperature
double average(const Field& field, const ParallelData parallel)
{
     double average = 0.0;

     for (int i = 1; i < field.nx + 1; i++) {
       for (int j = 1; j < field.ny + 1; j++) {
         average += field.temperature(i, j);
       }
     }

     MPI_Allreduce(MPI_IN_PLACE, &average, 1, MPI_DOUBLE, MPI_SUM, parallel.comm);
     average /= (field.nx_full * field.ny_full);

     return average;
}
