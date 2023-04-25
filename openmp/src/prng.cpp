#include <algorithm>
#include <array>
#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <errno.h>
#include <omp.h>
#include <stdio.h>
#include <time.h>
#include <vector>

#ifdef DEBUG
#define LOG_DEBUG(...) printf(__VA_ARGS__)
#else
#define LOG_DEBUG(...)                                                         \
  do {                                                                         \
  } while (0)
#endif

#define LOG(...) printf(__VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__);

using Data_t = double;
using ArrSize_t = uint64_t;
using BucketSize_t = uint64_t;
using BucketCount_t = uint64_t;
using ThreadCount_t = int32_t;
using SeriesCount_t = int32_t;

struct Args {
  ArrSize_t n_elems;
  BucketCount_t n_buckets;
  SeriesCount_t n_series;
};

struct ExpCfg {
  Args args;
  BucketSize_t bucket_size;
  SeriesCount_t series_id;
};

struct ExpResult {
  ExpCfg cfg;
  std::vector<BucketSize_t> bucket_sizes;

  static void print_header() {
    printf("sid,nelems,ebsize,nbuckets,bid,bsize\n");
  }

  void print_as_csv() const {
    for (BucketCount_t i_bucket = 0; i_bucket < bucket_sizes.size();
         ++i_bucket) {
      printf("%d,%ld,%ld,%ld,%ld,%ld\n", cfg.series_id, cfg.args.n_elems,
             cfg.bucket_size, cfg.args.n_buckets, i_bucket,
             bucket_sizes[i_bucket]);
    }
  }
};

inline static void init_rand_state(std::array<uint16_t, 3> &rstate) {
#ifdef DEBUG
  time_t ctime = 0;
#else
  time_t ctime = time(nullptr);
#endif

  uint16_t tid = 1;

  rstate[0] = (ctime + tid * 3 + 11) % UINT16_MAX;
  rstate[1] = (ctime + tid * 7 + 13) % UINT16_MAX;
  rstate[2] = (ctime + tid * 31 + 29) % UINT16_MAX;
}

static void parse_args(const int argc, char *argv[], Args &out);

static ExpResult draw_and_scatter(ExpCfg cfg) {
  ExpResult result;
  result.cfg = cfg;
  result.bucket_sizes = std::vector<BucketSize_t>(cfg.args.n_buckets);
  std::fill(std::begin(result.bucket_sizes), std::end(result.bucket_sizes), 0);

  std::array<uint16_t, 3> rstate;
  init_rand_state(rstate);

  Data_t data;
  for (ArrSize_t i = 0; i < cfg.args.n_elems; ++i) {
    data = erand48(rstate.data());
    result.bucket_sizes[static_cast<int>(data * cfg.args.n_buckets)]++;
  }

  return result;
}

int main(int argc, char *argv[]) {
  Args args;
  parse_args(argc, argv, args);

  ExpCfg cfg;
  cfg.args = args;
  cfg.bucket_size = args.n_elems / args.n_buckets;

  ExpResult::print_header();
  for (SeriesCount_t sid = 0; sid < args.n_series; ++sid) {
    cfg.series_id = sid;
    draw_and_scatter(cfg).print_as_csv();
  }

  return EXIT_SUCCESS;
}

static void parse_args(const int argc, char *argv[], Args &out) {
  assert(((argc == 4) &&
          "Three arguments are expected: n_elems, n_buckets, n_series"));

  out.n_elems = std::strtoull(argv[1], nullptr, 10);
  assert((errno == 0 && out.n_elems != 0 && "Correct conversion for n_elems"));

  out.n_buckets = std::strtol(argv[2], nullptr, 10);
  assert(
      (errno == 0 && out.n_buckets != 0 && "Correct conversion for n_buckets"));

  out.n_series = std::strtol(argv[3], nullptr, 10);
  assert(
      (errno == 0 && out.n_series != 0 && "Correct conversion for n_series"));
}
