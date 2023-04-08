#include <cstdlib>
#include <memory>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <vector>
#include <algorithm>
#include <cstdlib>

#define TIME_SCALE_FACTOR 1e6

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef int32_t i32;
typedef int64_t i64;
typedef double Data_t;

typedef struct Args {
  uint64_t array_size;
  int32_t n_threads;
} Args;

Args g_args;

static void parse_args(const int argc, char *argv[], Args *out) {
  assert(((argc == 2 || argc == 3) && "One or two arguments are expected"));
  out->array_size = std::strtoull(argv[1], nullptr, 10);
  assert((errno == 0 && "Correct conversion for array_size"));

  if (argc == 3) {
    out->n_threads = std::strtol(argv[2], nullptr, 10);
    assert((errno == 0 && "Correct conversion for n_threads"));
  } else {
    out->n_threads = -1;
  }
}

static void print_arr(const Data_t * const arr, const uint64_t size){
  for (u64 i = 0; i < size; ++i) {
    printf("%lf\n", arr[i]);
  }
}

// Should be executed only in OpenMP parallel execution context
inline static int32_t init_rand_state(uint16_t *rstate) {
  int32_t tid = omp_get_thread_num();
  rstate[0] = tid * 3 + 11;
  rstate[1] = tid * 7 + 13;
  rstate[2] = tid * 31 + 29;
  return tid;
}

void bucket_sort(Data_t *data, u64 size, i32 n_buckets) {
  u16 rstate[3];
  i32 tid = 1;
  i32 thread_range;
  std::vector<Data_t> buckets[n_buckets];

  #pragma omp parallel private(rstate, tid, buckets, thread_range) // ACHTUNG: what is copied here?
  {
    tid = init_rand_state(rstate);
    #pragma omp for schedule(static)
    for (u64 i = 0; i < size; ++i) {
      data[i] = erand48(rstate);
    }
    
    thread_range = 1 / (double)4; // ACHTUNG

    // #pragma omp for schedule(static)
    for (u64 i = 0; i < size; ++i) {
      if (data[i] >= tid * thread_range && data[i] < (tid + 1) * thread_range) {
        buckets[static_cast<int>(data[i] * n_buckets)].push_back(data[i]);
      }
    }

    for (const auto& bucket : buckets) {
      for (Data_t el : bucket) {
        printf("%lf ", el);
      }
      printf("\n");
    }

    #pragma omp for schedule(static)
    for (u64 i = 0; i < n_buckets; ++i) {
      // WE NEED TO SORT IT MANUALLY MOST LIKELY
      std::sort(buckets[i].begin(), buckets[i].end()); 
    }

    int i = 0; 
    for (auto& bucket : buckets) {
      for (Data_t el : bucket) {
        data[i++] = el;
      }
    }
  }

  print_arr(data, size);
}


int main(int argc, char * argv[]) {
  srand48(731);
  parse_args(argc, argv, &g_args);

  Data_t *data = new Data_t[g_args.array_size];
  assert((data != nullptr && "Memory allocated"));

  if (g_args.n_threads != -1) {
    omp_set_dynamic(0); // disable dynamic teams
    omp_set_num_threads(g_args.n_threads); // set upper bounds for threads
  }

  bucket_sort(data, g_args.array_size, 12);

  delete[] data;
	return 0;
}

