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
int n_blocks)
{
  int tx = get_global_id(0); 
  //for(unsigned int i=0;j< n_blocks;i++)
  for(unsigned int j=0;j< hA;j++)
  {
            C[tx * hA + j] = 12.0;
            for(unsigned int k=0;k< wA;k++)
            {
             // C[tx * hA + j] += +A[j * wA + k];// * B[tx * wA +k];
            }
  }

 
}
