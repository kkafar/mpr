#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>

char g_hostname[MPI_MAX_PROCESSOR_NAME];
int g_rank, g_size, g_hostname_len;

bool init_global_state(void) {
  assert((MPI_Comm_rank(MPI_COMM_WORLD, &g_rank) == MPI_SUCCESS) && "Valid rank returned");
  assert((MPI_Comm_size(MPI_COMM_WORLD, &g_size) == MPI_SUCCESS) && "Valid size returned");
  assert((MPI_Get_processor_name(g_hostname, &g_hostname_len) == MPI_SUCCESS) && "Valid processor name");
  return true;
}

inline bool teardown_global_state(void) {

  return true;
}

double estimate_pi(size_t point_count) {
  double x, y;
  size_t hit_count = 0;

  for (size_t i = 0; i < point_count; ++i) {
    // Hey! I want to avoid this division!
    // How do I generate 
    x = drand48();
    y = drand48();

    if (x * x + y * y <= 1) {
      ++hit_count;
    }
  }

  return (double)hit_count / point_count * 4; 
}

// Naive avg function
double daverage(double *arr, size_t size) {
  double acc = 0.0;  
  // This sum is potentially very unstable
  for (size_t i = 0; i < size; ++i) {
    acc += arr[i];
  }
  // This division is potentially very unstable
  return acc / size;
}

void dump_env(int argc, char *argv[]) {
  printf("--------------------------------\n");
  printf("Dumping environment of process %d running on %s\n", g_rank, g_hostname);
  for (int i = 0; i < argc; ++i) {
    printf("%d: %s\n", i, argv[i]);
  }
  printf("sizeof(long): %ld\n", sizeof(long));
  printf("sizeof(long long): %ld\n", sizeof(long long));
  printf("sizeof(double): %ld\n", sizeof(double));
  printf("sizeof(size_t): %ld\n", sizeof(size_t));
  printf("sizeof(uint64_t): %ld\n", sizeof(uint64_t));
  printf("sizeof(uintmax_t): %ld\n", sizeof(uintmax_t));
  printf("--------------------------------\n");
}


// Scatter i Gather -- do sprawdzenia!!!
// Jest też coś takeigo jak Reduce

int main(int argc, char * argv[]) {
  MPI_Init(&argc, &argv);
  init_global_state();
  srand48(time(NULL) + g_rank * 31);

  if (g_rank == 0) {
    dump_env(argc, argv);
  }

  double start_time, elapsed_time;

  MPI_Barrier(MPI_COMM_WORLD);

  start_time = MPI_Wtime() * 1e6;
  double pi_estimate = estimate_pi(100000);
  elapsed_time = MPI_Wtime() * 1e6 - start_time;

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

    double average = daverage(results, g_size);
    printf("%lf\n", average);

    // need to take average
  } else {
    MPI_Send(&pi_estimate, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
  }

  teardown_global_state();
  MPI_Finalize();
  return 0;
}

