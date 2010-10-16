include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

CUDA_add_executable( sandbox-cuda-matmul-v2 main.cxx matrix_sizes.h matrix_mult.h matrix_mult.cu )

target_link_libraries( sandbox-cuda-matmul-v2 ${Boost_LIBRARIES} )
