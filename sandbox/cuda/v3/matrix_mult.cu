#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// #include <cuda.h>
// #include <cuda_runtime.h>

#include "matrix_sizes.h"
#include "matrix_mult.h"

// CUDA Kernel
// Multiply two matrices A * B = C
__global__ void
cudakernel_matrix_mul( float* C, float* A, float* B, int wA, int wB)
{
  // Block index
  int bx = blockIdx.x;
  int by = blockIdx.y;

  // Thread index
  int tx = threadIdx.x;
  int ty = threadIdx.y;

  // Index of the first sub-matrix of A processed
  // by the block
  int aBegin = wA * BLOCK_SIZE * by;

  // Index of the last sub-matrix of A processed
  // by the block
  int aEnd   = aBegin + wA - 1;

  // Step size used to iterate through the
  // sub-matrices of A
  int aStep  = BLOCK_SIZE;

  // Index of the first sub-matrix of B processed
  // by the block
  int bBegin = BLOCK_SIZE * bx;

  // Step size used to iterate through the
  // sub-matrices of B
  int bStep  = BLOCK_SIZE * wB;

  float Csub = 0.;

  // Loop over all the sub-matrices of A and B
  // required to compute the block sub-matrix
  for (int a = aBegin, b = bBegin;
           a <= aEnd;
           a += aStep, b += bStep)
  {

    // Declaration of the shared memory array As
    // used to store the sub-matrix of A
    __shared__ float As[BLOCK_SIZE][BLOCK_SIZE];

    // Declaration of the shared memory array Bs
    // used to store the sub-matrix of B
    __shared__ float Bs[BLOCK_SIZE][BLOCK_SIZE];

    // Load the matrices from global memory
    // to shared memory; each thread loads
    // one element of each matrix
    As[ty][tx] = A[a + wA * ty + tx];
    Bs[ty][tx] = B[b + wB * ty + tx];

    // Synchronize to make sure the matrices
    // are loaded
    __syncthreads();

    // Multiply the two matrices together;
    // each thread computes one element
    // of the block sub-matrix
    Csub = 0.;
    for (int k = 0; k < BLOCK_SIZE; ++k)
      Csub += As[ty][k] * Bs[k][tx];

    // Synchronize to make sure that the preceding
    // computation is done before loading two new
    // sub-matrices of A and B in the next iteration
    __syncthreads();
  }

  // Write the block sub-matrix to device memory;
  // each thread writes one element
  int c = wB * BLOCK_SIZE * by + BLOCK_SIZE * bx;
  C[c + wB * ty + tx] = Csub;
}

  
void gpu_mat_mul(float* h_A, float* h_B, float* h_C )
{

    // allocate device memory
    float* d_A;
    float* d_B;
    float* d_C;

    unsigned int size_A = WA * HA;
    unsigned int size_B = WB * HB;
    unsigned int size_C = WC * HC;

    unsigned int mem_size_A = sizeof(float) * size_A;
    unsigned int mem_size_B = sizeof(float) * size_B;
    unsigned int mem_size_C = sizeof(float) * size_C;

    cudaMalloc((void**) &d_A, mem_size_A);
    cudaMalloc((void**) &d_B, mem_size_B);
    cudaMalloc((void**) &d_C, mem_size_C);

    // copy host memory to device*/
    cudaMemcpy(d_A, h_A, mem_size_A, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, mem_size_B, cudaMemcpyHostToDevice);
 
    // perform the calculation

    // setup execution parameters
    dim3 threads(BLOCK_SIZE, BLOCK_SIZE);
    dim3 grid(WC / threads.x, HC / threads.y);
 
    //   execute the kernel
    cudakernel_matrix_mul<<< grid, threads >>>(d_C, d_A, d_B, WA, WB);
 
    // copy result from device to host
    cudaMemcpy(h_C, d_C, mem_size_C, cudaMemcpyDeviceToHost);
 
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
}
