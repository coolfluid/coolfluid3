# reset the list of project files
SET ( CF_PROJECT_FILES "" CACHE INTERNAL "" FORCE )

# reset the list of orphan files
SET ( CF_ORPHAN_FILES  "" CACHE INTERNAL "" FORCE )

# user define that affects many search paths simultaneously
if ( DEFINED DEPS_ROOT )
  list(APPEND CMAKE_PREFIX_PATH ${DEPS_ROOT})
endif()

