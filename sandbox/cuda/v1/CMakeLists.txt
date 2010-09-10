  INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )

  CUDA_ADD_EXECUTABLE( cuda-matmul-v1 matrix_sizes.h matrix_mult_kernel.h matrix_mult.cu )
