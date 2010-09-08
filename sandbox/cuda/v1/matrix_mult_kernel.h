#ifndef _MATRIXMUL_KERNEL_H_
#define _MATRIXMUL_KERNEL_H_
 
#include <stdio.h>

#include "matrix_sizes.h"

// CUDA Kernel
__global__ void
matrixMul( float* C, float* A, float* B, int wA, int wB)
{
 
   // 2D Thread ID
   int tx = threadIdx.x;
   int ty = threadIdx.y;
 
   // value stores the element that is 
   // computed by the thread
   float value = 0;
   for (int i = 0; i < wA; ++i)
   {
      float elementA = A[ty * wA + i];
      float elementB = B[i * wB + tx];
      value += elementA * elementB;
   }
 
   // Write the matrix to device memory each 
   // thread writes one element
   C[ty * wA + tx] = value;
}
 
#endif // #ifndef _MATRIXMUL_KERNEL_H_



