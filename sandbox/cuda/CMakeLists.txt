if( CF_ENABLE_GPU AND CF_ENABLE_CUDA AND CUDA_FOUND )

add_subdirectory( v1 )

add_subdirectory( v2 )

add_subdirectory( v3 )

endif()
