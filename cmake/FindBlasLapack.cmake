# Confirm that liblapack library is installed
# This module defines
#   CF_HAVE_LAPACK
#   CF_HAVE_BLAS
#   CF_HAVE_BLASLAPACK
#   CF_BLASLAPACK_LIBRARIES
# Require both lapack and blas

if( NOT LAPACK_LIBRARIES )

# BLAS ############################

  if( NOT CF_HAVE_BLAS )

    if( EXISTS ${BLAS_DIR} )
      coolfluid_coolfluid_add_trial_library_path( ${BLAS_DIR} )
      coolfluid_coolfluid_add_trial_library_path( ${BLAS_DIR}/lib )
    endif()

    if( EXISTS $ENV{BLAS_HOME} )
      coolfluid_coolfluid_add_trial_library_path( $ENV{BLAS_HOME}/lib )
    endif()

    find_library(BLAS_LIBRARY blas ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
    find_library(BLAS_LIBRARY blas )

    if( BLAS_LIBRARY )
      set( CF_HAVE_BLAS 1 CACHE BOOL "Found BLAS library" )
    else()
      set( CF_HAVE_BLAS 0 )
    endif()

  endif()

  coolfluid_log( "CF_HAVE_BLAS: [${CF_HAVE_BLAS}]" )
  if(CF_HAVE_BLAS)
     coolfluid_log( "  BLAS_LIBRARY:     [${BLAS_LIBRARY}]" )
  endif(CF_HAVE_BLAS)

# LAPACK #########################

  if( NOT CF_HAVE_LAPACK )

    coolfluid_set_trial_include_path("") # clear include search path
    coolfluid_set_trial_library_path("") # clear library search path


    if( EXISTS ${LAPACK_DIR} )
      coolfluid_coolfluid_add_trial_library_path ( ${LAPACK_DIR}  )
      coolfluid_coolfluid_add_trial_library_path ( ${LAPACK_DIR}/lib )
    endif()

    if( EXISTS $ENV{LAPACK_HOME} )
      coolfluid_coolfluid_add_trial_library_path( $ENV{LAPACK_HOME} )
      coolfluid_coolfluid_add_trial_library_path( $ENV{LAPACK_HOME}/lib )
    endif()

    find_library(LAPACK_LIBRARY lapack ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
    find_library(LAPACK_LIBRARY lapack )

  endif()

  if( LAPACK_LIBRARY )
    set( CF_HAVE_LAPACK 1 CACHE BOOL "Found LAPACK library")
  else()
    set( CF_HAVE_LAPACK 0 )
  endif()


  coolfluid_log( "CF_HAVE_LAPACK: [${CF_HAVE_LAPACK}]" )
  if(CF_HAVE_LAPACK)
    coolfluid_log( "  LAPACK_LIBRARY:   [${LAPACK_LIBRARY}]" )
  endif(CF_HAVE_LAPACK)

# BOTH ###########################

  if( CF_HAVE_BLAS AND CF_HAVE_LAPACK )
    set( CF_BLASLAPACK_LIBRARIES   "${LAPACK_LIBRARY} ${BLAS_LIBRARY}" CACHE STRING "BLAS and LAPACK libraries")
    set( CF_HAVE_BLASLAPACK ON CACHE BOOL "Found BLAS and LAPACK libraries")
  endif()

  mark_as_advanced( LAPACK_LIBRARY BLAS_LIBRARY )

#################################

else()

  # user provided directly the libraries of LAPACK
  # TODO: test  that they actually work

  set(CF_HAVE_LAPACK       ON CACHE BOOL "Found LAPACK library")
  set(CF_HAVE_BLAS         ON CACHE BOOL "Found BLAS   library")
  set(CF_HAVE_BLASLAPACK   ON CACHE BOOL "Found BLAS and LAPACK libraries")

  set( CF_BLASLAPACK_LIBRARIES   "${LAPACK_LIBRARIES}" CACHE STRING "BLAS and LAPACK libraries")

  mark_as_advanced( CF_BLASLAPACK_LIBRARIES LAPACK_LIBRARIES )

  coolfluid_log( "CF_HAVE_BLASLAPACK: [${CF_HAVE_BLASLAPACK}]" )
  coolfluid_log( "  LAPACK_LIBRARIES: [${LAPACK_LIBRARIES}]" )

endif()

mark_as_advanced( CF_HAVE_LAPACK CF_HAVE_BLAS CF_HAVE_BLASLAPACK CF_BLASLAPACK_LIBRARIES )

