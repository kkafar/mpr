#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

char g_hostname[MPI_MAX_PROCESSOR_NAME];
int g_rank, g_size, g_hostname_len;

bool init_global_state(void) {
  assert((MPI_Comm_rank(MPI_COMM_WORLD, &g_rank) == MPI_SUCCESS) && "Valid rank returned");
  assert((MPI_Comm_size(MPI_COMM_WORLD, &g_size) == MPI_SUCCESS) && "Valid size returned");

  char *mem = (char *) malloc(sizeof(char) * MPI_MAX_PROCESSOR_NAME);
  if (mem == NULL) {
    printf("Failed to allocate memory for processor name in process %d\n", g_rank);
    return false;
  }
  assert((MPI_Get_processor_name(g_hostname, &g_hostname_len) == MPI_SUCCESS) && "Valid processor name");
  return true;
}

bool teardown_global_state(void) {
  return true;
}

double estimate_pi(size_t point_count) {
  double x, y;
  size_t hit_count = 0;

  for (size_t i = 0; i < point_count; ++i) {
    // Hey! I want to avoid this division!
    // How do I generate 
    x = (double)rand() / RAND_MAX;
    y = (double)rand() / RAND_MAX;

    if (x * x + y * y <= 1) {
      ++hit_count;
    }
  }

  return (double)hit_count / point_count * 4; 
}

int main(int argc, char * argv[]) {
  MPI_Init(&argc, &argv);
  init_global_state();

  srand(time(NULL));

  double pi_estimate = estimate_pi(100000);

  // printf("%lf\n", pi_estimate)MPI_Datatype datatype;

  MPI_Barrier(MPI_COMM_WORLD);

  if (g_rank == 0) {
    // I want to store them in the array,
    // and later try to avoid imprecisions 
    // resulting from adding numbers of vastly
    // different values
    double *results = (double *)calloc(g_size, sizeof(double));
    results[0] = pi_estimate;

    for (int worker_id = 1; worker_id < g_size; ++worker_id) {
      MPI_Recv(&results[worker_id], 1, MPI_DOUBLE, worker_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    for (size_t rid = 0; rid < g_size; ++rid) {
      printf("%ld: %lf\n", rid, results[rid]);
    }
  } else {
    MPI_Send(&pi_estimate, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
  }

  teardown_global_state();
  MPI_Finalize();
  return 0;
}

