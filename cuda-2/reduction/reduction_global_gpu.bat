nvcc -o reduction_global_gpu reduction_global.cpp reduction_global_kernel.cu -arch=sm_50 -allow-unsupported-compiler
