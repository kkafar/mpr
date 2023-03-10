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
  
  double start_time, end_time;

  // Synchronizujemy wszystkie procesy tego komunkatora
  MPI_Barrier(MPI_COMM_WORLD);

  start_time = MPI_Wtime();
  for (int i = 0; i < iteration_count; ++i) {
    if (rank == cping) {

      // Zaczynamy mierzyć czas
      // double send_time = MPI_Wtime();

      // Ping zaczyna
      MPI_Send(&payload, 1, MPI_BYTE, cpong, 0, MPI_COMM_WORLD);
      MPI_Recv(&payload, 1, MPI_BYTE, cpong, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      // double receive_time = MPI_Wtime();

      printf("Ping received data for %d time\n", i);
      
    } else if (rank == cpong) {
      MPI_Recv(&payload, 1, MPI_BYTE, cping, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("Pong received data for %d time\n", i);
      MPI_Send(&payload, 1, MPI_BYTE, cping, 0, MPI_COMM_WORLD);
    } else {
      printf("Dunno what happened\n");
    }
  }
  end_time = MPI_Wtime();

  printf("Time elapsed on %d: %lf\n", rank, end_time - start_time);

  MPI_Finalize();
  return 0;
}

