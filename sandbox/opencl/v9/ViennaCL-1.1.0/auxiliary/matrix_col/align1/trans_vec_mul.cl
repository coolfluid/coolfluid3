

__kernel void trans_vec_mul(
          __global const float * matrix,
          unsigned int matrix_rows,
          unsigned int matrix_cols,
          unsigned int matrix_internal_rows,
          unsigned int matrix_internal_cols,
          __global const float * vector,  
          __global float * result) 
{ 
  //row and col indicate indices within transposed matrix
  for (unsigned int row = get_global_id(0); row < matrix_cols; row += get_global_size(0))
  {
    float dot_prod2 = 0.0f;
    for (unsigned int col = 0; col < matrix_rows; ++col)
      dot_prod2 += matrix[row * matrix_internal_rows + col] * vector[col];
    result[row] = dot_prod2;
  }
}


