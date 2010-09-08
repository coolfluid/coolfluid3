
/* Multiply two matrices A * B = C */
 
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// #include <cuda.h>
// #include <cuda_runtime.h>

#include "matrix_sizes.h"
#include "matrix_mult_kernel.h"

/* Allocates a matrix with random float entries.*/
void randomInit(float* data, int size)
{
    for (int i = 0; i < size; ++i)
        data[i] = rand() / (float)RAND_MAX;
}
  
int
main(int argc, char** argv)
{

    /* set seed for rand()*/
    srand(2006);
 
    /* 1. allocate host memory for matrices A and B*/
    unsigned int size_A = WA * HA;
    unsigned int mem_size_A = sizeof(float) * size_A;
    float* h_A = (float*) malloc(mem_size_A);
 
    unsigned int size_B = WB * HB;
    unsigned int mem_size_B = sizeof(float) * size_B;
    float* h_B = (float*) malloc(mem_size_B);
 
    /* 2. initialize host memory*/
    randomInit(h_A, size_A);
    randomInit(h_B, size_B);
  
    /* 3. print out A and B*/
    printf("\n\nMatrix A\n");
    for( unsigned int i = 0; i < size_A; i++)
    {
       printf("%f ", h_A[i]);
       if(((i + 1) % WA) == 0)
          printf("\n");
    }
 
    printf("\n\nMatrix B\n");
    for( unsigned int i = 0; i < size_B; i++)
    {
       printf("%f ", h_B[i]);
       if(((i + 1) % WB) == 0)
          printf("\n");
    }
 
    /* 8. allocate device memory*/
    float* d_A;
    float* d_B;
    cudaMalloc((void**) &d_A, mem_size_A);
    cudaMalloc((void**) &d_B, mem_size_B);
 
    /* 9. copy host memory to device*/
    cudaMemcpy(d_A, h_A, mem_size_A, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, mem_size_B, cudaMemcpyHostToDevice);

 
    /* 4. allocate host memory for the result C*/
    unsigned int size_C = WC * HC;
    unsigned int mem_size_C = sizeof(float) * size_C;
    float* h_C = (float*) malloc(mem_size_C);
 
    /* 10. allocate device memory for the result*/
    float* d_C;
    cudaMalloc((void**) &d_C, mem_size_C);
 
    /* 5. perform the calculation    */
    /*    setup execution parameters */
    dim3 threads(BLOCK_SIZE, BLOCK_SIZE);
    dim3 grid(WC / threads.x, HC / threads.y);
 
    /*   execute the kernel */
    matrixMul<<< grid, threads >>>(d_C, d_A, d_B, WA, WB);
 
    /* 11. copy result from device to host */
    cudaMemcpy(h_C, d_C, mem_size_C,
               cudaMemcpyDeviceToHost);
 
    /* 6. print out the results */
    printf("\n\nMatrix C (Results)\n");
    for( unsigned  int i = 0; i < size_C; i++)
    {
       printf("%f ", h_C[i]);
       if(((i + 1) % WC) == 0)
          printf("\n");
    }
    printf("\n");
 
    /* 7. clean up memory */
    free(h_A);
    free(h_B);
    free(h_C);
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

}
