/* Main solver routines for heat equation solver */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "heat.h"

/* Exchange the boundary values */
void exchange(field *temperature, parallel_data *parallel)
{
    double *data;
    double *sbuf_up, *sbuf_down, *rbuf_up, *rbuf_down;

    // TODO start: implement halo exchange

    // You need to do a little bit of math to determine correct linear
    // indeces for inner boundaries and ghost layers

    data = temperature->data;

    // Send to up, receive from down
    sbuf_up = &data[temperature->ny + 2]; // Down data row (Down[1][0])
    rbuf_down = &data[(temperature->nx + 1) * (temperature->ny + 2)]; // Up ghost row (Up[Nx+1][0])
    
    MPI_Sendrecv(sbuf_up, temperature->ny + 2, MPI_DOUBLE,
              parallel->nup, 11,
              rbuf_down, temperature->ny + 2, MPI_DOUBLE,
              parallel->ndown, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Send to down, receive from up
    sbuf_down = &data[temperature->nx * (temperature->ny + 2)]; // Up data row (Up[Nx][0])
    rbuf_up = &data[0]; // Down ghost row (Down[0][0])

    MPI_Sendrecv(sbuf_down, temperature->ny + 2, MPI_DOUBLE,
                 parallel->ndown, 12,
                 rbuf_up, temperature->ny + 2, MPI_DOUBLE,
                 parallel->nup, 12, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // TODO end


}

/* Update the temperature values using five-point stencil */
void evolve(field *curr, field *prev, double a, double dt)
{
  double dx2, dy2;
  int nx, ny;
  double *currdata, *prevdata;

  /* HINT: to help the compiler do not access members of structures
   * within OpenACC parallel regions */
  currdata = curr->data;
  prevdata = prev->data;
  nx = curr->nx;
  ny = curr->ny;

  /* Determine the temperature field at next time step
   * As we have fixed boundary conditions, the outermost gridpoints
   * are not updated. */
  dx2 = prev->dx * prev->dx;
  dy2 = prev->dy * prev->dy;
  for (int i = 1; i < nx + 1; i++) {
    for (int j = 1; j < ny + 1; j++) {
            int ind = i * (ny + 2) + j;
            int ip = (i + 1) * (ny + 2) + j;
            int im = (i - 1) * (ny + 2) + j;
	    int jp = i * (ny + 2) + j + 1;
	    int jm = i * (ny + 2) + j - 1;
            currdata[ind] = prevdata[ind] + a * dt *
	      ((prevdata[ip] -2.0 * prevdata[ind] + prevdata[im]) / dx2 +
	       (prevdata[jp] - 2.0 * prevdata[ind] + prevdata[jm]) / dy2);
    }
  }

}
