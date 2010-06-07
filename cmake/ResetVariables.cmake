# reset the list of project files
SET ( CF_PROJECT_FILES "" CACHE INTERNAL "" FORCE )

# reset the list of orphan files
SET ( CF_ORPHAN_FILES  "" CACHE INTERNAL "" FORCE )

# user define that affects many search paths simultaneously
if ( DEFINED DEPS_ROOT )

  if ( NOT DEFINED BOOST_ROOT )
    SET ( BOOST_ROOT  ${DEPS_ROOT} CACHE INTERNAL "" )
  endif()

  if ( NOT DEFINED MPI_ROOT )
    SET ( MPI_ROOT  ${DEPS_ROOT} CACHE INTERNAL "" )
    SET ( MPI_HOME  ${DEPS_ROOT} CACHE INTERNAL "" )
  endif()

  if ( NOT DEFINED PARMETIS_ROOT )
    SET ( PARMETIS_ROOT  ${DEPS_ROOT} CACHE INTERNAL "" )
  endif()

  if ( NOT DEFINED CGNS_HOME )
    SET ( CGNS_HOME  ${DEPS_ROOT} CACHE INTERNAL "" )
  endif()

  if ( NOT DEFINED GOOGLE_PERFTOOLS_ROOT )
    SET ( GOOGLE_PERFTOOLS_ROOT  ${DEPS_ROOT} CACHE INTERNAL "" )
  endif()

endif()

