#include <iostream>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <cuda_runtime.h>
#include "kernel.h"
#include "dev_array.h"
#include <math.h>
#include <helper_timer.h>

using namespace std;

int main()
{
    // Perform matrix multiplication C = A*B
    // where A, B and C are NxN matrices
    int N = 16;
    int SIZE = N*N;

    // Allocate memory on the host
    vector<float> h_A(SIZE);
    vector<float> h_B(SIZE);
    vector<float> h_C(SIZE);

    // Initialize matrices on the host
    for (int i=0; i<N; i++){
        for (int j=0; j<N; j++){
            h_A[i*N+j] = sin(i);
            h_B[i*N+j] = cos(j);
        }
    }

    // Allocate memory on the device
    dev_array<float> d_A(SIZE);
    dev_array<float> d_B(SIZE);
    dev_array<float> d_C(SIZE);

    d_A.set(&h_A[0], SIZE);
    d_B.set(&h_B[0], SIZE);
    
    StopWatchInterface *timer;
    sdkCreateTimer(&timer);
  
    sdkStartTimer(&timer);

    matrixMultiplication(d_A.getData(), d_B.getData(), d_C.getData(), N);
    cudaDeviceSynchronize();

    d_C.get(&h_C[0], SIZE);
    cudaDeviceSynchronize();

    // Czas kopiowania danych do pamięci hosta wliczam w czas prowadzenia obliczen
    // bo jezeli nie bedziemy miec tych danych, to nic nam z tych obliczen.
    sdkStopTimer(&timer);
    float gpu_time = sdkGetTimerValue(&timer);
    sdkResetTimer(&timer);

    float *cpu_C;
    cpu_C=new float[SIZE];

    sdkStartTimer(&timer);
    // Now do the matrix multiplication on the CPU
    float sum;
    for (int row=0; row<N; row++){
        for (int col=0; col<N; col++){
            sum = 0.f;
            for (int n=0; n<N; n++){
                sum += h_A[row*N+n]*h_B[n*N+col];
            }
            cpu_C[row*N+col] = sum;
        }
    }

    sdkStopTimer(&timer);
    float cpu_time = sdkGetTimerValue(&timer);

    double err = 0;
    // Check the result and make sure it is correct
    for (int ROW=0; ROW < N; ROW++){
        for (int COL=0; COL < N; COL++){
            err += cpu_C[ROW * N + COL] - h_C[ROW * N + COL];
        }
    }

    cout << "Error: " << err << endl;
    cout << "GPU: " << gpu_time << "  CPU: " << cpu_time << '\n';


    sdkDeleteTimer(&timer);
    return 0;
}
