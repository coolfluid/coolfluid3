// OpenCL Kernel
__kernel void matrix_vector_mul( __global float* C, __global float* A, __global float* B, int wA, int hA, int n_blocks )
{
  for( unsigned int tx = get_group_id(0); tx <n_blocks; tx += get_num_groups(0) )
  {
      for(unsigned int j = get_local_id(0);j< hA;j+= get_local_size(0) )
      {
            float value = 0;
            for(unsigned int k=0;k< wA;k++)
            {
               value += A[j * wA + k] * B[tx * wA +k];
            }
            C[tx * hA + j] = value;
      }
  }
}
