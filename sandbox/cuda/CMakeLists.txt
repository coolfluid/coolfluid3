find_package(CUDA)

if(CUDA_FOUND)

#  set( CUDA_VERBOSE_BUILD 1)
#  coolfluid_debug_var(CUDA_VERSION_STRING)
#  coolfluid_debug_var(CUDA_TOOLKIT_ROOT_DIR)
#  coolfluid_debug_var(CUDA_INCLUDE_DIRS)

add_subdirectory( v1 )

add_subdirectory( v2 )

add_subdirectory( v3 )

endif()
