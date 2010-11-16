# reset the list of project files
set( CF_KERNEL_LIBS "" CACHE INTERNAL "" FORCE )

# reset the list of project files
set( CF_PROJECT_FILES "" CACHE INTERNAL "" FORCE )

# reset the list of orphan files
set( CF_ORPHAN_FILES  "" CACHE INTERNAL "" FORCE )

# user define that affects many search paths simultaneously
if( DEFINED DEPS_ROOT )
  list(APPEND CMAKE_PREFIX_PATH ${DEPS_ROOT})
endif()

# finding boost with BOOST_ROOT avoids conflicts with the system libraries
if( NOT DEFINED BOOST_ROOT )
  set( BOOST_ROOT  ${DEPS_ROOT} CACHE INTERNAL "" )
endif()

# finding MPI with MPI_HOME or MPI_ROOT avoids conflicts with the system libraries
if( NOT DEFINED MPI_ROOT )
  set( MPI_ROOT  ${DEPS_ROOT} CACHE INTERNAL "" )
  set( MPI_HOME  ${DEPS_ROOT} CACHE INTERNAL "" )
endif()
