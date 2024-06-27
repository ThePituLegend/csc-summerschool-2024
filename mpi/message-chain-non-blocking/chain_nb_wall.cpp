#include <cstdio>
#include <vector>
#include <mpi.h>

void print_ordered(double t);

int main(int argc, char *argv[])
{
    int i, rank, ntasks;
    constexpr int size = 10000000;
    std::vector<int> message(size);
    std::vector<int> receiveBuffer(size);
    MPI_Status status[2];
    MPI_Request request[2];

    double t0, t1;

    int source, destination;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Initialize buffers
    for (i = 0; i < size; i++) {
        message[i] = rank;
        receiveBuffer[i] = -1;
    }

    // TODO: Set source and destination ranks
    source = (rank - 1);
    destination = (rank + 1);

    // TODO: Treat boundaries with MPI_PROC_NULL
    if (rank == 0) source = MPI_PROC_NULL;
    else if (rank == ntasks - 1) destination = MPI_PROC_NULL;

    // Start measuring the time spent in communication
    MPI_Barrier(MPI_COMM_WORLD);
    t0 = MPI_Wtime();

    // TODO: Send messages
    MPI_Isend(message.data(), size, MPI_INT, destination, rank + 1, MPI_COMM_WORLD, &request[0]);

    printf("Sender: %d. Sent elements: %d. Tag: %d. Receiver: %d\n",
           rank, size, rank + 1, destination);

    // TODO: Receive messages
    MPI_Irecv(receiveBuffer.data(), size, MPI_INT, source, rank, MPI_COMM_WORLD, &request[1]);

    MPI_Waitall(2, request, status);

    int nrecv;
    MPI_Get_count(&status[1], MPI_INT, &nrecv);

    printf("Receiver: %d. Received elements: %d. First element %d.\n",
           rank, nrecv, receiveBuffer[0]);

    // Finalize measuring the time and print it out
    t1 = MPI_Wtime();
    MPI_Barrier(MPI_COMM_WORLD);
    fflush(stdout);

    print_ordered(t1 - t0);

    MPI_Finalize();
    return 0;
}

void print_ordered(double t)
{
    int i, rank, ntasks;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);

    if (rank == 0) {
        printf("Time elapsed in rank %2d: %6.3f\n", rank, t);
        for (i = 1; i < ntasks; i++) {
            MPI_Recv(&t, 1, MPI_DOUBLE, i, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Time elapsed in rank %2d: %6.3f\n", i, t);
        }
    } else {
        MPI_Send(&t, 1, MPI_DOUBLE, 0, 11, MPI_COMM_WORLD);
    }
}