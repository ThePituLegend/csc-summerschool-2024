// Main solver routines for heat equation solver

#include "heat.hpp"

// Update the temperature values using five-point stencil */
void evolve(Field& curr, Field& prev, const double a, const double dt)
{

  // Compilers do not necessarily optimize division to multiplication, so make it explicit
  auto inv_dx2 = 1.0 / (prev.dx * prev.dx);
  auto inv_dy2 = 1.0 / (prev.dy * prev.dy);

  // Help the compiler avoid being confused by the structs
  double *currdata = curr.temperature.data();
  double *prevdata = prev.temperature.data();

  int nx = curr.nx;
  int ny = curr.ny;

  int curr_size = curr.temperature.size();
  int prev_size = prev.temperature.size();

  // Determine the temperature field at next time step
  // As we have fixed boundary conditions, the outermost gridpoints
  // are not updated.
  #pragma omp target teams distribute map(tofrom: prevdata[0:prev_size], currdata[0:curr_size])
  for (int i = 1; i < nx + 1; i++) {
    #pragma omp parallel for
    for (int j = 1; j < ny + 1; j++) {
      int ind = i * (ny + 2) + j;
      int ip = (i + 1) * (ny + 2) + j;
      int im = (i - 1) * (ny + 2) + j;
      int jp = i * (ny + 2) + j + 1;
      int jm = i * (ny + 2) + j - 1;
      currdata[ind] = prevdata[ind] + a*dt*
      ((prevdata[ip] - 2.0*prevdata[ind] + prevdata[im]) * inv_dx2 +
      (prevdata[jp] - 2.0*prevdata[ind] + prevdata[jm]) * inv_dy2);
    }
  }

}

void enter_data(Field& curr, Field& prev)
{
   double* currdata = curr.temperature.data();
   double* prevdata = prev.temperature.data();

   int curr_size = curr.temperature.size();
   int prev_size = prev.temperature.size();

  #pragma omp target enter data map(to: prevdata[0:curr_size]) map(alloc: currdata[0:prev_size])
}

void exit_data(Field& curr, Field& prev)
{
   double* currdata = curr.temperature.data();
   double* prevdata = prev.temperature.data();

   int curr_size = curr.temperature.size();
   int prev_size = prev.temperature.size();

  #pragma omp target exit data map(from: prevdata[0:curr_size]) map(delete: currdata[0:prev_size])
}

void update_host(Field& curr)
{
  double* currdata = curr.temperature.data();
  int curr_size = curr.temperature.size();

  #pragma omp target update from(currdata[0:curr_size])
}
