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

#ifdef DEBUG
#define LOG_DEBUG(...) printf(__VA_ARGS__)
#else 
#define LOG_DEBUG(...) do {} while(0)
#endif

#define LOG(...) printf(__VA_ARGS__)

#define TIME_SCALE_FACTOR 1e6
#define TIME_MEASURE_BEGIN(x) ((x) = omp_get_wtime() * TIME_SCALE_FACTOR)
#define TIME_MEASURE_END(x) ((x) = omp_get_wtime() * TIME_SCALE_FACTOR - (x))

typedef double Data_t;

struct Args {
  using ArrSize_t = uint64_t;
  using BucketCount_t = uint64_t;
  using ThreadCount_t = int32_t;
  ArrSize_t array_size;
  BucketCount_t n_buckets;
  ThreadCount_t n_threads;
};

Args g_args;

static void parse_args(const int argc, char *argv[], Args *out);
static void dump_cfg();
static void print_arr(const Data_t * const arr, const Args::ArrSize_t size);
static void print_buckets(const std::vector<Data_t> *buckets, const Args::BucketCount_t n_buckets, const int32_t tid);
static bool summary(Data_t *data, const Args &args);

// Initializes random state for erand48 with arbitrary bytes.
// Assumes that rstate consists of 3 * 16 bytes.
inline static int32_t init_rand_state(uint16_t *rstate);

static void bucket_sort_1(Data_t *data, const Args args) {
  LOG_DEBUG("bucket_sort_1 called with data: %p, size: %ld, n_buckets: %ld, n_threads: %d\n", data, args.array_size, args.n_buckets, args.n_threads);

  uint16_t rstate[3];
  double thread_range;

  double total_time, draw_time, scatter_time, sort_time, gather_time;

  std::vector<std::vector<Data_t>> buckets(args.n_buckets);

  TIME_MEASURE_BEGIN(total_time);

  // Couldn't get the program to work with private buckets...
  // I'm also not sure whether there is any synchornization on shared variable,
  // could not f  #pragma omp single ind the info in reference.
  // #pragma omp parallel private(rstate, tid, buckets, thread_range)
  #pragma omp parallel private(rstate, thread_range) shared(buckets, args)
  {
    // threadprivate memory
    double p_draw_time = 0, p_scatter_time = 0, p_sort_time = 0, p_gather_time = 0;

    TIME_MEASURE_BEGIN(p_draw_time);
    const int tid = init_rand_state(rstate);

    #pragma omp for schedule(static)
    for (Args::ArrSize_t i = 0; i < args.array_size; ++i) {
      data[i] = erand48(rstate);
    }
    TIME_MEASURE_END(p_draw_time);

    
    TIME_MEASURE_BEGIN(p_scatter_time);
    thread_range = static_cast<double>(1.0) / static_cast<double>(g_args.n_threads);

    // Every thread reads entire array
    for (Args::ArrSize_t i = 0; i < args.array_size; ++i) {
      if (data[i] >= tid * thread_range && data[i] < (tid + 1) * thread_range) {
        buckets[(static_cast<int>(data[i] * args.n_buckets))].push_back(data[i]);
      }
    }
    TIME_MEASURE_END(p_scatter_time);

    TIME_MEASURE_BEGIN(p_sort_time);
    // Don't we need synchronization (barrier) here?
    // What happens if one thread is assigned a bucket which is still
    // being filled in earlier for? Documentation states that:
    // "There is a default barrier at the end of each worksharing construct unless
    // the `nowait` clause is present.", but I have not found anything on synchronization
    // before the worksharing construct.
    #pragma omp for schedule(static)
    for (Args::BucketCount_t i = 0; i < args.n_buckets; ++i) {
      std::sort(std::begin(buckets[i]), std::end(buckets[i])); 
    }
    TIME_MEASURE_END(p_sort_time);

    TIME_MEASURE_BEGIN(p_gather_time);
    // These are threadprivate, so initialization value holds for every thread
    Args::ArrSize_t prev_el_count = 0;
    Args::BucketCount_t last_j = 0;
    
    #pragma omp for schedule(static)
    for (Args::BucketCount_t i = 0; i < args.n_buckets; ++i) {
      // Small optimization: let's assume THIS thread has been assigned with buckets b, ..., b + n
      // To count how many elements there are in buckets 0, 1, ..., b - 1 we need to iterate over all of them,
      // however it is sufficient to do it once -- for bucket b. For buckets b + 1, ..., b + n we can use value
      // computed for previous bucket. 
      //
      // prev_el_count(i) = sum(el_count(j) for j in 0, ..., i - 1) when i = b
      // prev_el_count(i) = prev_el_count(i - 1) + el_count(i) when i > b
      for (Args::BucketCount_t j = last_j; j < i; ++j) {
        prev_el_count += buckets[j].size();
      }
      last_j = i;
      Args::ArrSize_t el_i = prev_el_count;
      for (Data_t el : buckets[i]) {
        data[el_i++] = el;
      }
    }
    TIME_MEASURE_END(p_gather_time);

    // Need to inject out the measured values into the outer scope
    // This is not part of the algorithm so we stop measuring time
    if (tid == 0) {
      draw_time = p_draw_time;
      scatter_time = p_scatter_time;
      sort_time = p_sort_time;
      gather_time = p_gather_time;
    }
  }
  TIME_MEASURE_END(total_time);

  LOG("total_time: %lf\ndraw_time: %lf\nscatter_time: %lf\nsort_time: %lf\ngather_time: %lf\n", total_time, draw_time, scatter_time, sort_time, gather_time);
}

int main(int argc, char * argv[]) {
  srand48(time(nullptr));
  parse_args(argc, argv, &g_args);
  dump_cfg();

  Data_t *data = new Data_t[g_args.array_size];
  assert((data != nullptr && "Memory allocated"));

  if (g_args.n_threads != -1) {
    omp_set_dynamic(0); // disable dynamic teams
    omp_set_num_threads(g_args.n_threads); // set upper bounds for threads
  }

  bucket_sort_1(data, g_args);
  
  if (!summary(data, g_args)) {
    LOG("ERROR: THE ARRAY IS NOT SORTED PROPERLY\n");
  }

  delete[] data;
	return 0;
}

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
  printf("n_elems: %ld\nn_threads: %d\nn_buckets: %ld\n", g_args.array_size, g_args.n_threads, g_args.n_buckets);
  printf("--------------------\n");
}

static void print_arr(const Data_t * const arr, const Args::ArrSize_t size) {
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

static void print_buckets(const std::vector<Data_t> *buckets, const Args::BucketCount_t n_buckets, const int32_t tid) {
  printf("------------\n");
  for (int i = 0; i < n_buckets; ++i) {
    printf("%d: ", i);
    for (Data_t el : buckets[i]) {
      printf("%lf ", el);
    }
    printf("\n");
  }
  printf("------------\n");
}

static bool summary(Data_t *data, const Args &args) {
  bool sorted = true;
  for (uint64_t i = 0; i < args.array_size; ++i) {
    if (i > 0 && data[i - 1] > data[i]) {
// #ifdef DEBUG
//       printf("  <--- X");
// #endif
      sorted = false;
    }
// #ifdef DEBUG
//     printf("\n%lf", data[i]);
// #endif
  }
// #ifdef DEBUG
//   printf("\n");
// #endif
  return sorted;
}

