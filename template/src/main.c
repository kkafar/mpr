#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int rank, size;

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    MPI_Status stat;

    if (size != 2) {
        if(rank == 0) {
            fprintf(stderr, "This prgoramm expect 2 MPI ranks %s\n", argv[0]);
        }
        MPI_Finalize();
        exit(0);
    }

    int i = 0;
    for(; i<=14; i++) {
        long int N = 1 << i;
        //long int N = 16;
        int buffSize = 5*((N*sizeof(double))+MPI_BSEND_OVERHEAD);
        double *A = (double*)malloc(buffSize);

        int j = 0;
        for(; j<N; j++) {
            A[j] = 0.0;
        }

        int tag1 = 10;
        int tag2 = 20;

        int loop_count = 50;

        MPI_Buffer_attach(&A, buffSize);

        double start_time, stop_time, elapsed_time;
        start_time = MPI_Wtime();

        int l = 1;
        for(; l<=loop_count; l++) {
            if(rank == 0) {
                MPI_Bsend(A, N, MPI_DOUBLE, 1, tag1, MPI_COMM_WORLD);
                MPI_Recv(A, N, MPI_DOUBLE, 1, tag2, MPI_COMM_WORLD, &stat);
            } else if(rank == 1) {
                MPI_Recv(A, N, MPI_DOUBLE, 0, tag1, MPI_COMM_WORLD, &stat);
                MPI_Bsend(A, N, MPI_DOUBLE, 0, tag2, MPI_COMM_WORLD);
            }
        }
        MPI_Buffer_detach(&A, &buffSize);

        stop_time = MPI_Wtime();
        elapsed_time = stop_time - start_time;

        long int num_B = 8*N;
        long int B_in_Mbit = 1 << 20;
        double num_Mbit = (double)num_B / (double)B_in_Mbit;
        double avg_time_per_transfer = elapsed_time / (2.0*(double)loop_count);

        if(rank == 0) {
            printf("Transfer size (B): %10li, Transfer Time (s): %15.9f, Bandwidth (Mbit/s):    %15.9f\n", num_B, avg_time_per_transfer, num_Mbit/avg_time_per_transfer);
        }

        free(A);
    }

    MPI_Finalize();
    return 0;
}

