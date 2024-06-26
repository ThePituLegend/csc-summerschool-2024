#include <cstdio>
#include <cmath>
#include <mpi.h>

constexpr int n = 840;

int main(int argc, char **argv)
{

    int rank, ntasks;
    int istart, istop;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
        printf("Computing approximation to pi with N=%d\n", n);

    // Split the computation between two tasks
    if (rank == 0)
    {
        istart = 1;
        istop = n / 2;
        printf("Rank %d: Computing from %d to %d\n", rank, istart, istop);
    }
    else
    {
        istart = n / 2 + 1;
        istop = n;
        printf("Rank %d: Computing from %d to %d\n", rank, istart, istop);
    }

    // Partial sum
    double pi = 0.0;
    for (int i = istart; i <= istop; i++)
    {
        double x = (i - 0.5) / n;
        pi += 1.0 / (1.0 + x * x);
    }

    // Sync and reduce
    if (rank == 1)
    {
        MPI_Send(&pi, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    else
    {
        double recv_pi;
        MPI_Recv(&recv_pi, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        pi += recv_pi;

        pi *= 4.0 / n;
        printf("Approximate pi=%18.16f (exact pi=%10.8f)\n", pi, M_PI);
    }

    MPI_Finalize();
}
