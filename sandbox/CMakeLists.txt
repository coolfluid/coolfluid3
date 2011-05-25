# coolfluid_find_orphan_files()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

###############################################################################
### test for boost::mpl
add_subdirectory( boost-mpl )

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

###############################################################################
### test for fexcept_t - currently some problems with clang in linux
add_subdirectory( fexcept )

###############################################################################
### test for XML layer
add_subdirectory( xml-layer )

###############################################################################
### test for weird boost::share_ptr behavior
add_subdirectory( boost-shared_ptr )

###############################################################################
### check that compiling with paraview works
add_subdirectory( paraview-deps )

###############################################################################
### linear solver trials
add_subdirectory( lss-trilinos )
