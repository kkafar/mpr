#include <array>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <cstdlib>

#define TIME_SCALE_FACTOR 1e6
#define TIME_MEASURE_BEGIN(x) ((x) = omp_get_wtime() * TIME_SCALE_FACTOR)
#define TIME_MEASURE_END(x) ((x) = omp_get_wtime() * TIME_SCALE_FACTOR - (x))

typedef double Data_t;

typedef struct Args {
  uint64_t array_size;
  int32_t n_threads;
  int32_t n_buckets;
} Args;

Args g_args;

static void parse_args(const int argc, char *argv[], Args *out) {
  assert(((argc == 2 || argc == 3 || argc == 4) && "One, two or three arguments are expected"));
  out->array_size = std::strtoull(argv[1], nullptr, 10);
  assert((errno == 0 && "Correct conversion for array_size"));

  if (argc >= 3) {
    out->n_threads = std::strtol(argv[2], nullptr, 10);
    assert((errno == 0 && "Correct conversion for n_threads"));
  } else {
    out->n_threads = -1;
  }

  if (argc >= 4) {
    out->n_buckets = std::strtol(argv[3], nullptr, 10);
    assert((errno == 0 && "Correct conversion for n_buckets"));
  } else {
    out->n_buckets = 12;
  }
}

static void dump_cfg() {
  printf("--------------------\n");
#ifdef _OPENMP
  printf("Execution context: OpenMP\n");
#else
  printf("Execution context: Sync\n");
#endif
  printf("n_elems: %ld\nn_threads: %d\nn_buckets: %d\n", g_args.array_size, g_args.n_threads, g_args.n_buckets);
  printf("--------------------\n");
}

static void print_arr(const Data_t * const arr, const uint64_t size) {
  for (uint64_t i = 0; i < size; ++i) {
    printf("%lf\n", arr[i]);
  }
}

#ifdef _OPENMP
// Should be executed only in OpenMP parallel execution context
inline static int32_t init_rand_state(uint16_t *rstate) {
  int32_t tid = omp_get_thread_num();

#ifdef DEBUG
  time_t ctime = 0;
#else
  time_t ctime = time(nullptr);
#endif

  rstate[0] = (ctime + tid * 3 + 11) % UINT16_MAX;
  rstate[1] = (ctime + tid * 7 + 13) % UINT16_MAX;
  rstate[2] = (ctime + tid * 31 + 29) % UINT16_MAX;
  return tid;
}
#endif // _OPENMP
 
static void print_buckets(const std::vector<Data_t> *buckets, const int32_t n_buckets) {
#ifdef _OPENMP
  #pragma omp barrier
#endif
  printf("------------\n");
  for (int i = 0; i < n_buckets; ++i) {
    printf("%d: ", i);
    for (Data_t el : buckets[i]) {
      printf("%lf ", el);
    }
    printf("\n");
  }
  printf("------------\n");
#ifdef _OPENMP
  #pragma omp barrier
#endif
}

static void bucket_sort_sync(Data_t *data, const uint64_t size, const int32_t n_buckets) {
  std::vector<Data_t> *buckets = new std::vector<Data_t>[n_buckets];
  for (uint64_t i = 0; i < size; ++i) {
    data[i] = drand48();
  }
  for (uint64_t i = 0; i < size; ++i) {
    buckets[static_cast<int>(data[i] * n_buckets)].push_back(data[i]);
  }
  for (int32_t i = 0; i < n_buckets; ++i) {
    std::sort(buckets[i].begin(), buckets[i].end());
  }
  uint64_t el_i = 0;
  for (int32_t bucket_i = 0; bucket_i < n_buckets; ++bucket_i) {
    for (Data_t el : buckets[bucket_i]) {
      data[el_i++] = el;
    }
  }
  print_arr(data, size);
  delete[] buckets;
}

#ifdef _OPENMP
static void bucket_sort_1(Data_t *data, const uint64_t size, const int32_t n_buckets) {
  printf("bucket_sort_1 called with data: %p, size: %ld, n_buckets: %d\n", data, size, n_buckets);

  uint16_t rstate[3];
  int tid = 1;
  double thread_range;
  double draw_time, scatter_time, sort_time, gather_time, total_time;

  // std::vector<Data_t> *buckets = new std::vector<Data_t>[n_buckets];
  std::vector<std::vector<Data_t>> buckets(n_buckets);

  printf("Before parrallel section\n");

  TIME_MEASURE_BEGIN(total_time);

  // Couldn't get the program to work with private buckets...
  // I'm also not sure whether there is any synchornization on shared variable,
  // could not find the info in reference.
  // #pragma omp parallel private(rstate, tid, buckets, thread_range)
  #pragma omp parallel private(rstate, tid, thread_range, draw_time, scatter_time, sort_time, gather_time, total_time)
  {
    TIME_MEASURE_BEGIN(draw_time);
    tid = init_rand_state(rstate);

    #pragma omp for schedule(static)
    for (uint64_t i = 0; i < size; ++i) {
      data[i] = erand48(rstate);
    }

    TIME_MEASURE_END(draw_time);
    
    TIME_MEASURE_BEGIN(scatter_time);
    thread_range = static_cast<double>(1.0) / static_cast<double>(g_args.n_threads);

    if (tid == 0) {
      print_buckets(buckets.data(), n_buckets);
    }

    // Every thread reads entire array
    for (uint64_t i = 0; i < size; ++i) {
      if (data[i] >= tid * thread_range && data[i] < (tid + 1) * thread_range) {
        buckets.at(static_cast<int>(data[i] * n_buckets)).push_back(data[i]);
      }
    }

    TIME_MEASURE_END(scatter_time);

    // Printing for debug purposes
    if (tid == 0) {
      print_buckets(buckets.data(), n_buckets);
    }

    TIME_MEASURE_BEGIN(sort_time);
    // Don't we need synchronization (barrier) here?
    // What happens if one thread is assigned a bucket which is still
    // being filled in earlier for? Documentation states that:
    // "There is a default barrier at the end of each worksharing construct unless
    // the `nowait` clause is present.", but I have not found anything on synchronization
    // before the worksharing construct.
    #pragma omp for schedule(static)
    for (uint64_t i = 0; i < n_buckets; ++i) {
      // WE NEED TO SORT IT MANUALLY MOST LIKELY
      std::vector<Data_t> &bucket = buckets[i];
      std::sort(bucket.begin(), bucket.end()); 
    }
    TIME_MEASURE_END(sort_time);

    print_buckets(buckets.data(), n_buckets);

    TIME_MEASURE_BEGIN(gather_time);
    // Each thread needs to count how many elements in lower buckets
    if (tid == 0) {
      int j = 0;
      for (uint64_t i = 0; i < n_buckets; ++i) {
        for (Data_t el : buckets[i]) {
          data[j++] = el;
        }
      }
    }
    TIME_MEASURE_END(gather_time);
  }
  TIME_MEASURE_END(total_time);

  // delete[] buckets;
  print_arr(data, size);
}
#endif // _OPENMP


int main(int argc, char * argv[]) {
  srand48(time(nullptr));
  parse_args(argc, argv, &g_args);
  dump_cfg();

  Data_t *data = new Data_t[g_args.array_size];
  assert((data != nullptr && "Memory allocated"));

#ifdef _OPENMP
  if (g_args.n_threads != -1) {
    omp_set_dynamic(0); // disable dynamic teams
    omp_set_num_threads(g_args.n_threads); // set upper bounds for threads
  }

  g_args.n_threads = omp_get_num_threads();

  bucket_sort_1(data, g_args.array_size, g_args.n_buckets);
#else
  bucket_sort_sync(data, g_args.array_size, g_args.n_buckets);
#endif

  delete[] data;
	return 0;
}

