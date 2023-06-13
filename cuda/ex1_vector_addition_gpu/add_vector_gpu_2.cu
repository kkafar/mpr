#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// #include "gputimer.h"
struct GpuTimer {
  cudaEvent_t start;
  cudaEvent_t stop;

  GpuTimer() {
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
  }

  ~GpuTimer() {
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
  }

  void Start() { cudaEventRecord(start, 0); }

  void Stop() { cudaEventRecord(stop, 0); }

  float Elapsed() {
    float elapsed;
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&elapsed, start, stop);
    return elapsed;
  }
};

#define N 1048576 // 512, 1024, ... 16384, 32768, 65536, 131072

void host_add(int *a, int *b, int *c, int size) {
  for (int idx = 0; idx < size; idx++)
    c[idx] = a[idx] + b[idx];
}

__global__ void device_add(int *a, int *b, int *c) {

  int index = threadIdx.x + blockIdx.x * blockDim.x;
  c[index] = a[index] + b[index];
}

// basically just fills the array with index.
void fill_array(int *data, int size) {
  for (int i = 0; i < size; ++i) {
    data[i] = i;
  }
}

void print_output(int *a, int *b, int *c, int size) {
  for (int idx = 0; idx < size; idx++)
    printf("\n %d + %d  = %d", a[idx], b[idx], c[idx]);
}
int main(void) {
  std::cout << "Starting the run...\n";

  std::vector<int> arr_sizes;
  std::vector<int> thread_counts{32, 64, 128, 256, 512};

  int base = 512;
  while (base <= N) {
    arr_sizes.push_back(base);
    base *= 2;
  }

  printf("arrsize,nblocks,nthreads,time\n");

  for (int i = arr_sizes.size() - 1; i >= 0; --i) {
    for (int i_thread = 0; i_thread < thread_counts.size(); ++i_thread) {
      int *a, *b, *c;
      int *d_a, *d_b, *d_c; // device copies of a, b, c
      int threads_per_block = 0, no_of_blocks = 0;
      GpuTimer timer;
      
      int n_elems = arr_sizes[i];
      int size = n_elems * sizeof(int);

      // Alloc space for host copies of a, b, c and setup input values
      a = (int *)malloc(size);
      fill_array(a, n_elems);
      b = (int *)malloc(size);
      fill_array(b, n_elems);
      c = (int *)malloc(size);

      // Alloc space for device copies of a, b, c
      cudaMalloc((void **)&d_a, size);
      cudaMalloc((void **)&d_b, size);
      cudaMalloc((void **)&d_c, size);

      // Copy inputs to device
      cudaMemcpy(d_a, a, size, cudaMemcpyHostToDevice);
      cudaMemcpy(d_b, b, size, cudaMemcpyHostToDevice);

      threads_per_block = thread_counts[i_thread];
      no_of_blocks = arr_sizes[i] / threads_per_block;
      timer.Start();
      device_add<<<no_of_blocks, threads_per_block>>>(d_a, d_b, d_c);
      timer.Stop();

      // Copy result back to host
      cudaMemcpy(c, d_c, size, cudaMemcpyDeviceToHost);

      // print_output(a,b,c);
      printf("%d,%d,%d,%f\n", n_elems, no_of_blocks, threads_per_block, timer.Elapsed());
      // printf("N = %d; no_of_blocks = %d; Elapsed time = %f ms\n", N,
      // no_of_blocks,
      //        timer.Elapsed());

      free(a);
      free(b);
      free(c);
      cudaFree(d_a);
      cudaFree(d_b);
      cudaFree(d_c);
    }
  }

  return 0;
}
