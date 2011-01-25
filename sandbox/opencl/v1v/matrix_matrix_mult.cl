// kernel.cl
// Multiply two matrices A * B = C
// Device code.


// OpenCL Kernel
__kernel void
matrix_mul(__global float* C,
__global float* A,
__global float* B,
int wA,
int hA,
int wB,
int n_blocks)
{
  int tx = get_global_id(0); 
  int ty = get_global_id(1); 
  for(unsigned int j=0;j< hA;j++)
  {
            int elemC = tx * ( hA*wB ) + j*wB + ty;
            C[elemC] = 0.0;
            for(unsigned int k=0;k< wA;k++)
            {
              int elemA = j * wA + k;
              int elemB = tx * ( wA*wB ) + k*wB + j;
              C[elemC] += +A[elemA] * B[elemB];
            }
  }
 
}
