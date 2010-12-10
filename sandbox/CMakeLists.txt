# coolfluid_find_orphan_files()

###############################################################################
### test for boost::lambda
add_subdirectory( boost-lambda )

###############################################################################
### test for boost::bind
add_subdirectory( boost-bind )

###############################################################################
### test for boost::multi_array
add_subdirectory( boost-multiarray )

###############################################################################
### test for boost::ublas
add_subdirectory( boost-ublas )

###############################################################################
### test for CGAL functions
add_subdirectory( cgal )

###############################################################################
### test for CUDA
if( CF_ENABLE_GPU AND CF_ENABLE_CUDA )
  add_subdirectory( cuda )
endif()

###############################################################################
### test for OpenCL
if( CF_ENABLE_GPU AND CF_ENABLE_OPENCL )
  add_subdirectory( opencl )
endif()

###############################################################################
### test for nullptr
add_subdirectory( nullptr )


