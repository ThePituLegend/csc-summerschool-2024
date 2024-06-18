#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int i, rank, ntasks;
    int size = 10000000;
    int *message;
    int *receiveBuffer;
    MPI_Status statuses[2];
    MPI_Request requests[2];

    double t0, t1;

    int source, destination;
    int count;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* Allocate message buffers */
    message = malloc(sizeof(int) * size);
    receiveBuffer = malloc(sizeof(int) * size);
    /* Initialize message */
    for (i = 0; i < size; i++) {
        message[i] = rank;
    }

    /* Set source and destination ranks */
    if (rank < ntasks - 1) {
        destination = rank + 1;
    } else {
        destination = MPI_PROC_NULL;
    }
    if (rank > 0) {
        source = rank - 1;
    } else {
        source = MPI_PROC_NULL;
    }

    /* Start measuring the time spend in communication */
    MPI_Barrier(MPI_COMM_WORLD);
    t0 = MPI_Wtime();

    /* Describe the receiving requests */
    MPI_Recv_init(receiveBuffer, size, MPI_INT, source, MPI_ANY_TAG,
                  MPI_COMM_WORLD, &requests[0]);
    /* Describe the sending requests */
    MPI_Send_init(message, size, MPI_INT, destination, rank + 1,
                  MPI_COMM_WORLD, &requests[1]);

    /* Start communication */
    MPI_Startall(2, requests);

    /* Blocking wait for messages */
    MPI_Waitall(2, requests, statuses);

    /* Finalize measuring the time and print it out */
    t1 = MPI_Wtime();

    /* Use status parameter to find out the no. of elements received */
    MPI_Get_count(&statuses[0], MPI_INT, &count);
    printf("Sender: %d. Sent elements: %d. Tag: %d. Receiver: %d\n",
           rank, size, rank + 1, destination);
    printf("Receiver: %d. Received elements: %d. Tag %d. Sender: %d.\n",
           rank, count, statuses[0].MPI_TAG, statuses[0].MPI_SOURCE);

    MPI_Barrier(MPI_COMM_WORLD);
    fflush(stdout);

    printf("Time elapsed in rank %2d: %6.3f\n", rank, t1 - t0);

    free(message);
    free(receiveBuffer);
    MPI_Finalize();
    return 0;
}
