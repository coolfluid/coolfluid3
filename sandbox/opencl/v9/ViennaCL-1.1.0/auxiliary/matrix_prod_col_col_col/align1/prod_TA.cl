// file automatically generated - do not edit!
// matrix-matrix multiplication C = A^T * B
// matrix layouts: C...col_major, A...col_major, B...col_major
__kernel void prod_TA(
          __global const float * A,
          unsigned int A_rows,
          unsigned int A_cols,
          unsigned int A_internal_rows,
          unsigned int A_internal_cols,
          __global const float * B,  
          unsigned int B_rows,
          unsigned int B_cols,
          unsigned int B_internal_rows,
          unsigned int B_internal_cols,
          __global float * C,
          unsigned int C_rows,
          unsigned int C_cols,
          unsigned int C_internal_rows,
          unsigned int C_internal_cols,
          __local float * bufA,
          __local float * bufB) 
{ 
  int block_size = get_local_size(0);
  int row_block_id = get_group_id(0);
  int col_block_id = get_group_id(1);
  int row_thread_id = get_local_id(0);
  int col_thread_id = get_local_id(1);
  int aBegin = row_block_id * block_size * A_internal_rows;
  int aStep = block_size;
  int bBegin = col_block_id * block_size * B_internal_rows;
  int bStep = block_size;
  int block_num = A_rows / block_size;
  if (block_num * block_size != A_rows)
    ++block_num;
  float Csub = 0;
  int aOffset = row_thread_id * A_internal_rows + col_thread_id;
  int bOffset = row_thread_id + col_thread_id * B_internal_rows;
  for (int block = 0;
           block < block_num;
           ++block)
  {
    if (block * block_size + col_thread_id < A_rows && get_global_id(0) < A_cols)
      bufA[row_thread_id * block_size + col_thread_id] = A[aBegin + aOffset];
    else
      bufA[row_thread_id * block_size + col_thread_id] = 0;
    if ( (block * block_size + row_thread_id < B_rows) && get_global_id(1) < B_cols )
      bufB[row_thread_id * block_size + col_thread_id] = B[bBegin + bOffset]; 
    else
      bufB[row_thread_id * block_size + col_thread_id] = 0;
    barrier(CLK_LOCAL_MEM_FENCE);
    for (int k = 0; k < block_size; ++k)
      Csub += bufA[row_thread_id * block_size + k] * bufB[k * block_size + col_thread_id];
    barrier(CLK_LOCAL_MEM_FENCE);
    aBegin += aStep;
    bBegin += bStep;
  }
  if (get_global_id(0) < A_cols && get_global_id(1) < B_cols)
    C[get_global_id(0) + get_global_id(1) * C_internal_rows] = Csub;
}
