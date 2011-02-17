


__kernel void vec_mul(
          __global const float * matrix,
          unsigned int matrix_rows,
          unsigned int matrix_cols,
          unsigned int matrix_internal_rows,
          unsigned int matrix_internal_cols,
          __global const float * vector,  
          __global float * result) 
{ 
  for (unsigned int row = get_global_id(0); row < matrix_rows; row += get_global_size(0))
  {
    float dot_prod = 0.0f;
    for (unsigned int col = 0; col < matrix_cols; ++col)
      dot_prod += matrix[row*matrix_internal_cols + col] * vector[col];
    result[row] = dot_prod;
  }
}


