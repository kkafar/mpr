#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

typedef uintmax_t Size_t;

typedef struct ProcessArgs {
  Size_t point_count;
} ProcessArgs;

char g_hostname[MPI_MAX_PROCESSOR_NAME];
int g_rank, g_size, g_hostname_len;
ProcessArgs g_pargs;

inline bool init_global_state(void) {
  assert((MPI_Comm_rank(MPI_COMM_WORLD, &g_rank) == MPI_SUCCESS) && "Valid rank returned");
  assert((MPI_Comm_size(MPI_COMM_WORLD, &g_size) == MPI_SUCCESS) && "Valid size returned");
  assert((MPI_Get_processor_name(g_hostname, &g_hostname_len) == MPI_SUCCESS) && "Valid processor name");
  return true;
}

inline bool teardown_global_state(void) {
  return true;
}

double estimate_pi(Size_t point_count) {
  double x, y;
  Size_t hit_count = 0;

  for (Size_t i = 0; i < point_count; ++i) {
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
double daverage(double *arr, Size_t size) {
  double acc = 0.0;  
  // This sum is potentially very unstable
  for (Size_t i = 0; i < size; ++i) {
    acc += arr[i];
  }
  // This division is potentially very unstable
  return acc / size;
}

void dump_env(int argc, char *argv[]) {
  printf("--------------------------------\n");
  printf("Host: %s\n", g_hostname);
  printf("Process: %d\nArgs:\n", g_rank);
  for (int i = 0; i < argc; ++i) {
    printf("\t%d: %s\n", i, argv[i]);
  }
  printf("sizeof(long): %ld\n", sizeof(long));
  printf("sizeof(long long): %ld\n", sizeof(long long));
  printf("sizeof(double): %ld\n", sizeof(double));
  printf("sizeof(size_t): %ld\n", sizeof(size_t));
  printf("sizeof(uint64_t): %ld\n", sizeof(uint64_t));
  printf("sizeof(uintmax_t): %ld\n", sizeof(uintmax_t));
  printf("Point count: %ld\n", g_pargs.point_count);
  printf("--------------------------------\n");
}

bool parse_args(int argc, char *argv[], ProcessArgs *output) {
  // Right now the program expects exactly one argument - point count
  if (argc != 2) {
    output->point_count = 1e5;
    return false;
  }
  Size_t total_point_count = strtoll(argv[1], NULL, 10);
  assert((total_point_count > 0) && "Point count must be > 0");

  output->point_count = total_point_count / g_size;

  return true;
}

int main(int argc, char * argv[]) {
  MPI_Init(&argc, &argv);
  init_global_state();
  srand48(time(NULL) + g_rank * 31);

  parse_args(argc, argv, &g_pargs);

  if (g_rank == 0) {
    dump_env(argc, argv);
  }

  double start_time, elapsed_time;
  // double *reduce_buffer = NULL;
  double reduce_buffer;

  MPI_Barrier(MPI_COMM_WORLD);

  if (g_rank == 0) {
    start_time = MPI_Wtime() * 1e6;
  }

  // I parse the args for the second time here to have
  // some sequential operations here
  parse_args(argc, argv, &g_pargs);

  // if (g_rank == 0) {
  //   reduce_buffer = (double *) calloc(g_size, sizeof(double));
  //   if (reduce_buffer == NULL) {
  //     printf("Failed to allocate buffer of size %ld\n", g_size * sizeof(double));
  //     goto CLEANUP;
  //   }
  // }

  double pi_estimate = estimate_pi(g_pargs.point_count);

  MPI_Reduce(&pi_estimate, &reduce_buffer, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  
  if (g_rank == 0) {
    double average = reduce_buffer / g_size;
    elapsed_time = MPI_Wtime() * 1e6 - start_time;
    printf("%lf,%lf\n", average, elapsed_time);
  }

CLEANUP:
  teardown_global_state();
  MPI_Finalize();
  return 0;
}

