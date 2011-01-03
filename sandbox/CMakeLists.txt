# coolfluid_find_orphan_files()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

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
add_subdirectory( cuda )

###############################################################################
### test for OpenCL
add_subdirectory( opencl )

###############################################################################
### test for Zoltan
add_subdirectory( zoltan )

###############################################################################
### test for nullptr
add_subdirectory( nullptr )

###############################################################################
### test for mpi
add_subdirectory( mpi )


