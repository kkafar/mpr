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
#include <string>

#ifdef DEBUG
#define LOG_DEBUG(...) printf(__VA_ARGS__)
#else 
#define LOG_DEBUG(...) do {} while(0)
#endif

#define LOG(...) printf(__VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__);

#define TIME_SCALE_FACTOR 1e6
#define TIME_MEASURE_BEGIN(x) ((x) = omp_get_wtime() * TIME_SCALE_FACTOR)
#define TIME_MEASURE_END(x) ((x) = omp_get_wtime() * TIME_SCALE_FACTOR - (x))

struct ExpResult;
struct ExpCfg;

using Data_t = double;
using ArrSize_t = uint64_t;
using BucketSize_t = uint64_t;
using BucketCount_t = uint64_t;
using ThreadCount_t = int32_t;
using SeriesCount_t = int32_t;
using ExpFnHandle_t = ExpResult (*)(Data_t *, const ExpCfg);

enum ExpType {
  Async,
  Sync,
};

struct Args {
  ArrSize_t arr_size;
  BucketCount_t n_buckets;
  ThreadCount_t n_threads;
  SeriesCount_t n_series;
  ExpType exp_type;
} g_args;

struct ExpCfg {
  Args args;
  BucketSize_t bucket_size;
  SeriesCount_t series_id;
};

struct ExpResult {
  ExpCfg cfg;
  double total_time;
  double draw_time;
  double scatter_time;
  double sort_time;
  double gather_time;

  static void print_header() {
    printf("sid,arrsize,bsize,nthreads,total,draw,scatter,sort,gather\n");
  }
  void print_as_csv() const {
    printf("%d,%ld,%ld,%d,%lf,%lf,%lf,%lf,%lf\n",
        cfg.series_id, cfg.args.arr_size, cfg.bucket_size, cfg.args.n_threads, total_time,
        draw_time, scatter_time, sort_time, gather_time);
  }
};

static void parse_args(const int argc, char *argv[], Args *out);
static void dump_cfg();
static void print_arr(const Data_t * const arr, const ArrSize_t size);
static void print_buckets(const std::vector<std::vector<Data_t>> &buckets);
static bool summary(Data_t *data, const Args &args);

// Initializes random state for erand48 with arbitrary bytes.
// Assumes that rstate consists of 3 * 16 bytes.
inline static int32_t init_rand_state(uint16_t *rstate);

static ExpResult bucket_sort_sync(Data_t *data, const ExpCfg cfg) {
  ExpResult result;
  result.cfg = cfg;

  std::vector<std::vector<Data_t>> buckets(cfg.args.n_buckets);

  TIME_MEASURE_BEGIN(result.total_time);

  TIME_MEASURE_BEGIN(result.draw_time);
  for (ArrSize_t i = 0; i < cfg.args.arr_size; ++i)
    data[i] = drand48();
  TIME_MEASURE_END(result.draw_time);

  TIME_MEASURE_BEGIN(result.scatter_time);
  for (ArrSize_t i = 0; i < cfg.args.arr_size; ++i)
    buckets[(static_cast<int>(data[i] * cfg.args.n_buckets))].push_back(data[i]);
  TIME_MEASURE_END(result.scatter_time);

  TIME_MEASURE_BEGIN(result.sort_time);
  for (auto &bucket : buckets)
    std::sort(std::begin(bucket), std::end(bucket));
  TIME_MEASURE_END(result.sort_time);

  TIME_MEASURE_BEGIN(result.gather_time);
  ArrSize_t el_i = 0;
  for (auto &bucket : buckets)
    for (Data_t el : bucket)
      data[el_i++] = el;
  TIME_MEASURE_END(result.gather_time);

  TIME_MEASURE_END(result.total_time);

  return result;
}

static ExpResult bucket_sort_1(Data_t *data, const ExpCfg cfg) {
  LOG_DEBUG("bucket_sort_1 called with data: %p, size: %ld, n_buckets: %ld, n_threads: %d\n", data, cfg.args.arr_size, cfg.args.n_buckets, cfg.args.n_threads);

  uint16_t rstate[3];

  ExpResult result{};
  result.cfg = cfg;

  std::vector<std::vector<Data_t>> buckets(cfg.args.n_buckets);

  TIME_MEASURE_BEGIN(result.total_time);
  #pragma omp parallel private(rstate) shared(data, buckets, cfg)
  {
    // threadprivate memory
    ExpResult p_result{};

    TIME_MEASURE_BEGIN(p_result.draw_time);
    const int tid = init_rand_state(rstate);

    #pragma omp for schedule(static)
    for (ArrSize_t i = 0; i < cfg.args.arr_size; ++i) {
      data[i] = erand48(rstate);
    }
    TIME_MEASURE_END(p_result.draw_time);
    
    TIME_MEASURE_BEGIN(p_result.scatter_time);

    // I'm depending here on implementation of schedule(static). Basically this fragment of code
    // mimics static scheduling, so that bucket-to-thread assigment in the for loop below is the same
    // as in omp managed for loop with schedule(static).
    // Such approach allows me to resign from synchornizing all threads on barrier before `sort` section,
    // as it guarantees that if bucket `b` is being sorted all numbers that ought to be inside `b` are already
    // in, because the thread that sorts the bucket was responsible for putting them inside the bucket.
    BucketCount_t lower, upper, buckets_per_thread, remaining_buckets, used_remainder, bucket_for_data;
    buckets_per_thread = cfg.args.n_buckets / cfg.args.n_threads;
    remaining_buckets = cfg.args.n_buckets % cfg.args.n_threads;
    used_remainder = std::min(static_cast<BucketCount_t>(tid), remaining_buckets);
    lower = tid * buckets_per_thread + used_remainder;
    upper = (tid + 1) * buckets_per_thread + used_remainder - (remaining_buckets <= tid);
    
    ArrSize_t thread_offset = tid * (cfg.args.arr_size / cfg.args.n_threads);
    for (ArrSize_t i = thread_offset; i < cfg.args.arr_size; ++i) {
      bucket_for_data = static_cast<int>(data[i + thread_offset] * cfg.args.n_buckets);
      if (bucket_for_data >= lower && bucket_for_data <= upper) {
        buckets[bucket_for_data].push_back(data[i + thread_offset]);
      }
    }
    for (ArrSize_t i = 0; i < thread_offset; ++i) {
      bucket_for_data = static_cast<int>(data[i + thread_offset] * cfg.args.n_buckets);
      if (bucket_for_data >= lower && bucket_for_data <= upper) {
        buckets[bucket_for_data].push_back(data[i + thread_offset]);
      }
    }
    TIME_MEASURE_END(p_result.scatter_time);

    // #pragma omp barrier

    TIME_MEASURE_BEGIN(p_result.sort_time);
    #pragma omp for schedule(static)
    for (BucketCount_t i = 0; i < cfg.args.n_buckets; ++i) {
      std::sort(std::begin(buckets[i]), std::end(buckets[i])); 
    }
    TIME_MEASURE_END(p_result.sort_time);

    TIME_MEASURE_BEGIN(p_result.gather_time);
    // These are threadprivate, so initialization value holds for every thread
    ArrSize_t prev_el_count = 0;
    BucketCount_t last_j = 0;
    
    #pragma omp for schedule(static)
    for (BucketCount_t i = 0; i < cfg.args.n_buckets; ++i) {
      // Small optimization: let's assume THIS thread has been assigned with buckets b, ..., b + n
      // To count how many elements there are in buckets 0, 1, ..., b - 1 we need to iterate over all of them,
      // however it is sufficient to do it once -- for bucket b. For buckets b + 1, ..., b + n we can use value
      // computed for previous bucket. 
      //
      // prev_el_count(i) = sum(el_count(j) for j in 0, ..., i - 1) when i = b
      // prev_el_count(i) = prev_el_count(i - 1) + el_count(i) when i > b
      for (BucketCount_t j = last_j; j < i; ++j) {
        prev_el_count += buckets[j].size();
      }
      last_j = i;
      ArrSize_t el_i = prev_el_count;
      for (Data_t el : buckets[i]) {
        data[el_i++] = el;
      }
    }
    TIME_MEASURE_END(p_result.gather_time);

    // Need to inject out the measured values into the outer scope
    // This is not part of the algorithm so we stop measuring time
    if (tid == 0) {
      result.draw_time = p_result.draw_time;
      result.scatter_time = p_result.scatter_time;
      result.sort_time = p_result.sort_time;
      result.gather_time = p_result.gather_time;
    }
  }
  TIME_MEASURE_END(result.total_time);
  return result;
}

int main(int argc, char * argv[]) {
  parse_args(argc, argv, &g_args);

#ifdef DEBUG
  srand48(31);
  dump_cfg();
#else 
  srand48(time(nullptr));
#endif

  Data_t *data = new Data_t[g_args.arr_size];
  assert((data != nullptr && "Memory allocated"));

  if (g_args.n_threads != -1) {
    if (omp_get_max_threads() < g_args.n_threads) {
      LOG_ERROR("Demanded number of threads: %d is greater than available: %d\n", g_args.n_threads, omp_get_max_threads());
      std::exit(EXIT_FAILURE);
    }
    omp_set_dynamic(0); // disable dynamic teams
    omp_set_num_threads(g_args.n_threads); // set upper bounds for threads
  }
  
  ExpCfg cfg;
  cfg.args = g_args;
  cfg.bucket_size = g_args.arr_size / g_args.n_buckets;

  ExpFnHandle_t experiment_fn;
  if (g_args.exp_type == ExpType::Async) {
    experiment_fn = bucket_sort_1;
  } else {
    experiment_fn = bucket_sort_sync;
  }

  ExpResult::print_header();
  for (SeriesCount_t sid = 0; sid < g_args.n_series; ++sid) {
    cfg.series_id = sid;
    experiment_fn(data, cfg).print_as_csv();
    if (!summary(data, g_args)) {
      LOG_ERROR("ERROR: THE ARRAY IS NOT SORTED PROPERLY\n");
    }
  }

  delete[] data;
	return 0;
}

static void parse_args(const int argc, char *argv[], Args *out) {
  assert(((argc == 5 || argc == 6) && "Four or five arguments are expected"));

  out->arr_size = std::strtoull(argv[1], nullptr, 10);
  assert((errno == 0 && out->arr_size != 0 && "Correct conversion for arr_size"));

  out->n_threads = std::strtol(argv[2], nullptr, 10);
  assert((errno == 0 && out->n_threads != 0 && "Correct conversion for n_threads"));

  out->n_buckets = std::strtol(argv[3], nullptr, 10);
  assert((errno == 0 && out->n_buckets != 0 && "Correct conversion for n_buckets"));

  out->n_series = std::strtol(argv[4], nullptr, 10);
  assert((errno == 0 && out->n_series != 0 && "Correct conversion for n_series"));

  out->exp_type = ExpType::Async;
  if (argc == 6) {
    std::string exp_type_str{argv[5]};
    if (exp_type_str == "sync") {
      out->exp_type = ExpType::Sync;
    } else if (exp_type_str == "async") {
      out->exp_type = ExpType::Async;
    } else {
      LOG_ERROR("ERROR: Invalid experiment type %s\n", argv[5]);
      std::exit(EXIT_FAILURE);
    }
  }

  assert((out->n_buckets >= out->n_threads && "n_buckets must be >= n_threads"));
}

static void dump_cfg() {
  printf("--------------------\n");
#ifdef _OPENMP
  printf("Execution context: OpenMP\n");
#else
  printf("Execution context: Sync\n");
#endif
  printf("n_elems: %ld\nn_threads: %d\nn_buckets: %ld\n", g_args.arr_size, g_args.n_threads, g_args.n_buckets);
  printf("--------------------\n");
}

static void print_arr(const Data_t * const arr, const ArrSize_t size) {
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

static void print_buckets(const std::vector<std::vector<Data_t>> &buckets) {
  printf("------------\n");
  for (int i = 0; i < buckets.size(); ++i) {
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
  for (uint64_t i = 0; i < args.arr_size; ++i) {
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

