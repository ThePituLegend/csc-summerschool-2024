#include <cstdio>
#include <mpi.h>
#include <omp.h>

int main(int argc, char *argv[])
{
    int my_id, omp_rank;
    int provided, required=MPI_THREAD_FUNNELED;

    /* TODO: Initialize MPI with thread support. */
    MPI_Init_thread(&argc, &argv, required, &provided);

    /* TODO: Find out the MPI rank and thread ID of each thread and print
     *       out the results. */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    
    #pragma omp parallel private(omp_rank)
    {
        omp_rank = omp_get_thread_num();
        printf("Hello from MPI rank %d, thread %d\n", my_id, omp_rank);
    }
    /* TODO: Investigate the provided thread support level. */
    if (my_id == 0)
    {
        printf("Available thread support levels:\n");
        printf("\tMPI_THREAD_SINGLE: %d\n", MPI_THREAD_SINGLE);
        printf("\tMPI_THREAD_FUNNELED: %d\n", MPI_THREAD_FUNNELED);
        printf("\tMPI_THREAD_SERIALIZED: %d\n", MPI_THREAD_SERIALIZED);
        printf("\tMPI_THREAD_MULTIPLE: %d\n", MPI_THREAD_MULTIPLE);
        printf("Thread support level provided: %d\n", provided);
    }
    
    MPI_Finalize();
    return 0;
}
