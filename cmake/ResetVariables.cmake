# reset the list of kernel libs

set( CF3_KERNEL_LIBS "" CACHE INTERNAL "" )
set( CF3_PLUGIN_LIST "" CACHE INTERNAL "" )

# reset the list of project n orphan files

set( CF3_PROJECT_FILES "" CACHE INTERNAL "" )
set( CF3_ORPHAN_FILES  "" CACHE INTERNAL "" )

# reset enabled/disabled tests

set( CF3_ENABLED_UTESTS  "" CACHE INTERNAL "" )
set( CF3_DISABLED_UTESTS "" CACHE INTERNAL "" )
set( CF3_ENABLED_ATESTS  "" CACHE INTERNAL "" )
set( CF3_DISABLED_ATESTS "" CACHE INTERNAL "" )
set( CF3_ENABLED_PTESTS  "" CACHE INTERNAL "" )
set( CF3_DISABLED_PTESTS "" CACHE INTERNAL "" )

# user define that affects many search paths simultaneously

if( DEFINED DEPS_ROOT )

  list(APPEND CMAKE_PREFIX_PATH ${DEPS_ROOT})

  # finding boost with BOOST_ROOT avoids conflicts with the system libraries
  if( NOT DEFINED BOOST_ROOT )
    set( BOOST_ROOT  ${DEPS_ROOT} CACHE INTERNAL "" )
  endif()

  # finding MPI with MPI_HOME or MPI_ROOT avoids conflicts with the system libraries
  if( NOT DEFINED MPI_ROOT )
    set( MPI_ROOT  ${DEPS_ROOT} CACHE INTERNAL "" )
    set( MPI_HOME  ${DEPS_ROOT} CACHE INTERNAL "" )
  endif()

endif()

