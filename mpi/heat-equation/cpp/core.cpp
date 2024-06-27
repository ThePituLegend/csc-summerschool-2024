// Main solver routines for heat equation solver

#include <mpi.h>

#include "heat.hpp"

void evolve(Field & curr, int i, int j, const Field & prev, const double a, const double dt, double inv_dx2, double inv_dy2)
{
  curr(i, j) = prev(i, j) + a * dt * (
	           ( prev(i + 1, j) - 2.0 * prev(i, j) + prev(i - 1, j) ) * inv_dx2 +
	           ( prev(i, j + 1) - 2.0 * prev(i, j) + prev(i, j - 1) ) * inv_dy2
            );
}

void exchange_begin(Field& field, ParallelData& parallel)
{
    // Send to up, receive from down
    double* sbuf_up = field.temperature.data(1, 0);
    double* rbuf_down  = field.temperature.data(field.nx + 1, 0);
    MPI_Isend(sbuf_up, field.ny + 2, MPI_DOUBLE,
              parallel.nup, 11, parallel.comm, &parallel.nb_reqs[0]);
    MPI_Irecv(rbuf_down, field.ny + 2, MPI_DOUBLE,
              parallel.ndown, 11, parallel.comm, &parallel.nb_reqs[1]);

    // Send to down, receive from up
    double* sbuf_down = field.temperature.data(field.nx, 0);
    double* rbuf_up = field.temperature.data();
    MPI_Isend(sbuf_down, field.ny + 2, MPI_DOUBLE,
              parallel.ndown, 12, parallel.comm, &parallel.nb_reqs[2]);
    MPI_Irecv(rbuf_up, field.ny + 2, MPI_DOUBLE,
              parallel.nup, 12, parallel.comm, &parallel.nb_reqs[3]);
}

void exchange_end(ParallelData& parallel)
{
    MPI_Waitall(4, parallel.nb_reqs, MPI_STATUSES_IGNORE);
}

// Update the inner temperature values using five-point stencil */
void evolve_inner(Field& curr, const Field& prev, const double a, const double dt)
{
  // Compilers do not necessarily optimize division to multiplication, so make it explicit
  auto inv_dx2 = 1.0 / (prev.dx * prev.dx);
  auto inv_dy2 = 1.0 / (prev.dy * prev.dy);

  // Determine the temperature field at next time step
  // As we have fixed boundary conditions, the outermost gridpoints
  // are not updated.
  for (int i = 2; i < curr.nx; i++) { // Skip the ghost layers
    for (int j = 1; j < curr.ny + 1; j++) {
      evolve(curr, i, j, prev, a, dt, inv_dx2, inv_dy2);
    }
  }
}

// Update the outer temperature values using five-point stencil */
void evolve_outer(Field& curr, const Field& prev, const double a, const double dt)
{
  // Compilers do not necessarily optimize division to multiplication, so make it explicit
  auto inv_dx2 = 1.0 / (prev.dx * prev.dx);
  auto inv_dy2 = 1.0 / (prev.dy * prev.dy);

  // Determine the temperature field at next time step
  // As we have fixed boundary conditions, the outermost gridpoints
  // are not updated.
  for (int j = 1; j < curr.ny + 1; j++) {
    evolve(curr, 1, j, prev, a, dt, inv_dx2, inv_dy2);        // First ghost layer
    evolve(curr, curr.nx, j, prev, a, dt, inv_dx2, inv_dy2);  // Last ghost layer
  }
}
