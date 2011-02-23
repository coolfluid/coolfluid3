// OpenCL Kernel
__kernel void matrix_vector_mult_advanced(__global float* C, __global float* A, __global float* B, int wA, int hA, int n_variables, int n_blocks )
{
 for( unsigned int tx = get_group_id(0); tx <n_blocks; tx += get_num_groups(0) )
 {
      for(unsigned int j = get_local_id(0);j< hA;j+= get_local_size(0) )
      {
            for( unsigned int k = 0; k < n_variables; k++ )
            {
                unsigned int elemC = tx * ( n_variables * hA ) + j * n_variables + k;
                
                float value = 0;
                
                for( unsigned int l = 0; l < wA; l++ )
                {
                    unsigned int elemA = j * wA + l;
                    unsigned int elemB = tx * ( n_variables * wA ) + l * n_variables + k;
                    
                    value += A[elemA] * B[elemB];
                }
                C[elemC] = value;
            }
      }
  }
}
