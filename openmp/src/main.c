#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#define TIME_SCALE_FACTOR 1e6

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef int32_t i32;
typedef int64_t i64;
typedef double Data_t;

typedef struct Args {
  u64 array_size;
  i32 n_threads;
} Args;

Args g_args;

void parse_args(const int argc, char *argv[], Args *out) {
  assert(((argc == 2 || argc == 3) && "One or two arguments are expected"));

  out->array_size = strtoull(argv[1], NULL, 10);
  assert((errno == 0 && "Correct conversion for array_size"));

  if (argc == 3) {
    out->n_threads = strtol(argv[2], NULL, 10);
    assert((errno == 0 && "Correct conversion for n_threads"));
  } else {
    out->n_threads = -1;
  }
}

void print_arr(const Data_t *arr, const u64 size){
  for (u64 i = 0; i < size; ++i) {
    printf("%lf\n", arr[i]);
  }
}

void experiment_static(Data_t *data, u64 size) {
  double time_s, time_e;
	u16 rstate[3];
  i32 tid;

  time_s = omp_get_wtime() * TIME_SCALE_FACTOR;
  #pragma omp parallel private(rstate, tid)
  {
    tid = omp_get_thread_num();
		rstate[0] = tid;
		rstate[1] = tid * 7;
		rstate[2] = tid * 31;
    #pragma omp for  schedule(static, 2)
    for (u64 i = 0; i < size; ++i) {
      data[i] = erand48(rstate);
    }
  }
  time_e = omp_get_wtime() * TIME_SCALE_FACTOR - time_s;

  printf("static,%d,2,%ld,%lf\n", g_args.n_threads, g_args.array_size, time_e);

  time_s = omp_get_wtime() * TIME_SCALE_FACTOR;
  #pragma omp parallel private(rstate, tid)
  {
    tid = omp_get_thread_num();
		rstate[0] = tid;
		rstate[1] = tid * 7;
		rstate[2] = tid * 31;
    #pragma omp for  schedule(static)
    for (u64 i = 0; i < size; ++i) {
      data[i] = erand48(rstate);
    }
  }
  time_e = omp_get_wtime() * TIME_SCALE_FACTOR - time_s;

  printf("static,%d,auto,%ld,%lf\n", g_args.n_threads, g_args.array_size, time_e);
}

void experiment_dynamic(Data_t *data, u64 size) {
  double time_s, time_e;
	u16 rstate[3];
  i32 tid;

  time_s = omp_get_wtime() * TIME_SCALE_FACTOR;
  #pragma omp parallel private(rstate, tid)
  {
    tid = omp_get_thread_num();
		rstate[0] = tid;
		rstate[1] = tid * 7;
		rstate[2] = tid * 31;
    #pragma omp for  schedule(dynamic, 2)
    for (u64 i = 0; i < size; ++i) {
      data[i] = erand48(rstate);
    }
  }
  time_e = omp_get_wtime() * TIME_SCALE_FACTOR - time_s;

  printf("dynamic,%d,2,%ld,%lf\n", g_args.n_threads, g_args.array_size, time_e);

  time_s = omp_get_wtime() * TIME_SCALE_FACTOR;
  #pragma omp parallel private(rstate, tid)
  {
    tid = omp_get_thread_num();
		rstate[0] = tid;
		rstate[1] = tid * 7;
		rstate[2] = tid * 31;
    #pragma omp for  schedule(dynamic)
    for (u64 i = 0; i < size; ++i) {
      data[i] = erand48(rstate);
    }
  }
  time_e = omp_get_wtime() * TIME_SCALE_FACTOR - time_s;

  printf("dynamic,%d,auto,%ld,%lf\n", g_args.n_threads, g_args.array_size, time_e);
}

void experiment_guided(Data_t *data, u64 size) {
  double time_s, time_e;
	u16 rstate[3];
  i32 tid;

  time_s = omp_get_wtime() * TIME_SCALE_FACTOR;
  #pragma omp parallel private(rstate, tid)
  {
    tid = omp_get_thread_num();
		rstate[0] = tid;
		rstate[1] = tid * 7;
		rstate[2] = tid * 31;
    #pragma omp for  schedule(guided, 2)
    for (u64 i = 0; i < size; ++i) {
      data[i] = erand48(rstate);
    }
  }
  time_e = omp_get_wtime() * TIME_SCALE_FACTOR - time_s;

  printf("guided,%d,2,%ld,%lf\n", g_args.n_threads, g_args.array_size, time_e);

  time_s = omp_get_wtime() * TIME_SCALE_FACTOR;
  #pragma omp parallel private(rstate, tid)
  {
    tid = omp_get_thread_num();
		rstate[0] = tid;
		rstate[1] = tid * 7;
		rstate[2] = tid * 31;
    #pragma omp for  schedule(guided)
    for (u64 i = 0; i < size; ++i) {
      data[i] = erand48(rstate);
    }
  }
  time_e = omp_get_wtime() * TIME_SCALE_FACTOR - time_s;

  printf("guided,%d,auto,%ld,%lf\n", g_args.n_threads, g_args.array_size, time_e);
}


int main(int argc, char * argv[]) {
  srand48(731);
  parse_args(argc, argv, &g_args);

  Data_t *data = (Data_t *) malloc(sizeof(Data_t) * g_args.array_size);
  assert((data != NULL && "Memory allocated"));

  if (g_args.n_threads != -1) {
    omp_set_dynamic(0); // disable dynamic teams
    omp_set_num_threads(g_args.n_threads); // set upper bounds for threads
  }

  printf("type,threads,chunk,size,time\n");

  experiment_static(data, g_args.array_size);
  experiment_dynamic(data, g_args.array_size);
  experiment_guided(data, g_args.array_size);

	return 0;
}

