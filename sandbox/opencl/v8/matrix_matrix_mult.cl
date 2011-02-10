// OpenCL Kernel
__kernel void matrix_matrix_mul( __global float* C, __global float* A, __global float* B, int wA, int hA, int wB, int n_blocks )
{
  int n_elements_B = wB * wA;
  int n_elements_C = wB * hA;
  
  for( unsigned int tx = get_group_id(0); tx <n_blocks; tx += get_num_groups(0) )
  {
      for(unsigned int j = get_local_id(0);j< hA;j+= get_local_size(0) )
      {  
          for( int k = 0; k < wB; k++ )  
          {  
               unsigned int elemC = tx * n_elements_C + j*wB + k;  
               float value = 0.0;  
               for(unsigned int l=0;l< wA;l++)  
               {  
                   unsigned int elemA = j * wA + l;  
                   unsigned int elemB = tx * n_elements_B + l * wB + k;  
                   value += A[elemA] * B[elemB];  
               }  
               
               C[elemC] = value;
          }      
      }  
  }
 
}
