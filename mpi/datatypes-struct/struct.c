#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>


int main(int argc, char *argv[])
{
  int n=1000, reps=10000;

  typedef struct {
    float coords[3];
    int charge;
    char label[2];
  } particle;

  particle particles[n];

  int i, j, rank;
  double t1, t2;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Fill in some values for the particles
  if (rank == 0) {
    for (i=0; i < n; i++) {
      for (j=0; j < 3; j++) {
        particles[i].coords[j] = (float)rand()/(float)RAND_MAX*10.0;
      }
      particles[i].charge = 54;
      strcpy(particles[i].label, "Xe");
    }
  }

  // Define datatype for the struct
  MPI_Datatype particle_dt;
  int blocklens[3] = {3, 1, 2};
  MPI_Datatype types[3] = {MPI_FLOAT, MPI_INT, MPI_CHAR};

  // Get displacements
  MPI_Aint displ[3];
  MPI_Get_address(&particles[0].coords, &displ[0]);
  MPI_Get_address(&particles[0].charge, &displ[1]);
  MPI_Get_address(&particles[0].label, &displ[2]);

  // Make relative displacements
  displ[2] -= displ[0];
  displ[1] -= displ[0];
  displ[0] = 0;

  // Create datatype
  MPI_Type_create_struct(3, blocklens, displ, types, &particle_dt);
  MPI_Type_commit(&particle_dt);

  // Check extent
  MPI_Aint lb, extent;
  MPI_Type_get_extent(particle_dt, &lb, &extent);

  if (extent != sizeof(particle[0])) {
    MPI_Datatype tmp_dt = particle_dt;
    MPI_Type_create_resized(tmp_dt, 0, sizeof(particles[0]), &particle_dt);
    MPI_Type_commit(&particle_dt);
    MPI_Type_free(&tmp_dt);
  }


  // Communicate using the created particletype
  // Multiple sends are done for better timing
  t1 = MPI_Wtime();
  if (rank == 0) {
    for (i=0; i < reps; i++) {
      MPI_Send(particles, n, particle_dt, 1, i, MPI_COMM_WORLD);
    }
  } else if (rank == 1) {
    for (i=0; i < reps; i++) {
      MPI_Recv(particles, n, particle_dt, 0, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }
  t2 = MPI_Wtime();

  printf("Time: %i, %e \n", rank, (t2-t1)/(double)reps);
  printf("Check: %i: %s %f %f %f \n", rank, particles[n-1].label,
          particles[n-1].coords[0], particles[n-1].coords[1],
          particles[n-1].coords[2]);

  // Free datatype
  MPI_Type_free(&particle_dt);

  MPI_Finalize();
  return 0;
}
