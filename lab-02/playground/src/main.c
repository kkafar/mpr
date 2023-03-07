#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

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

  srand(time(NULL));

  double pi_estimate = estimate_pi(100000);

  printf("%lf\n", pi_estimate);

  MPI_Finalize();
  return 0;
}

