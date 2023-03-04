#include <mpi.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char * argv[]) {
  MPI_Init(&argc, &argv);

  int rank, size;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (size != 2) {
    printf("We need exactly two processes to complete this task. Detected %d processes\n", size);
    MPI_Finalize();
    return 1;
  }

  const int data_size = 34;
  char data[data_size];
  // Sender
  if (rank == 0) {
    sprintf(data, "Hello world from process of rank 0");
    MPI_Send(data, data_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
  } else if (rank == 1) {
    MPI_Recv(data, data_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Process %d received following message:\n\n%s\n\n", rank, data);
  }

  MPI_Finalize();
  return 0; 
}

