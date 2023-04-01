#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define TIME_SCALE_FACTOR 1e6

typedef uint64_t u64;
typedef double Data_t;

typedef struct Args {
  u64 array_size;
} Args;

Args g_args;

void parse_args(const int argc, char *argv[], Args *out) {
  assert((argc == 2 && "Single argument is expected"));

  // Assuming happy path here
  out->array_size = strtoull(argv[1], NULL, 10);
}

void print_arr(const Data_t *arr, const u64 size){
  for (u64 i = 0; i < size; ++i) {
    printf("%lf\n", arr[i]);
  }
}


int main(int argc, char * argv[]) {
  srand48(731);
  parse_args(argc, argv, &g_args);
  
  int tid, n_threads;
  double time_s, time_e;
  Data_t *data = (Data_t *) malloc(sizeof(Data_t) * g_args.array_size);

  printf("Filling array with random numbers\n");
  
  time_s = omp_get_wtime() * TIME_SCALE_FACTOR;
  #pragma omp parallel private(tid, n_threads)
  {
    tid = omp_get_thread_num();
    printf("Thread %d is executing\n", tid);

    if (tid == 0) {
      n_threads = omp_get_num_threads();
      printf("Total number of used threads: %d\n", n_threads);
    }
    #pragma omp for
    for (u64 i = 0; i < g_args.array_size; ++i) {
      data[i] = drand48();
    }
  }
  time_e = omp_get_wtime() * TIME_SCALE_FACTOR - time_s;

  printf("Elapsed time: %lf us\n", time_e);

	return 0;
}

