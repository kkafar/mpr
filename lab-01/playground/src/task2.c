#include <mpi.h>
#include <stdio.h>


int main(int argc, char * argv[]) {
  MPI_Init(&argc, &argv);

  int rank, size;
  const int cping = 0;
  const int cpong = 1;
  const char communication_type[] = "Standard";

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  char payload = 'a';

  const int iteration_count = 10;

  for (int i = 0; i < iteration_count; ++i) {
    if (rank == cping) {
      // Ping zaczyna
      MPI_Send(&payload, 1, MPI_BYTE, cpong, 0, MPI_COMM_WORLD);
      MPI_Recv(&payload, 1, MPI_BYTE, cpong, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("Ping received data for %d time\n", i);
      
    } else if (rank == cpong) {
      MPI_Recv(&payload, 1, MPI_BYTE, cping, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("Pong received data for %d time\n", i);
      MPI_Send(&payload, 1, MPI_BYTE, cping, 0, MPI_COMM_WORLD);
    } else {
      printf("Dunno what happened\n");
    }
  }

  MPI_Finalize();
  return 0;
}

