include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

CUDA_add_executable( sandbox-cuda-matmul-v1 matrix_sizes.h matrix_mult_kernel.h matrix_mult.cu )
