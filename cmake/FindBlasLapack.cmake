# Confirm that liblapack library is installed
# This module defines
#   CF3_HAVE_LAPACK
#   CF3_HAVE_BLAS
#   CF3_HAVE_BLASLAPACK
#   BLASLAPACK_LIBRARIES
# Require both lapack and blas

if( NOT LAPACK_LIBRARIES )

# BLAS ############################

  if( NOT CF3_HAVE_BLAS )

    if( EXISTS ${BLAS_DIR} )
      coolfluid_add_trial_library_path( ${BLAS_DIR} )
      coolfluid_add_trial_library_path( ${BLAS_DIR}/lib )
    endif()

    if( EXISTS $ENV{BLAS_HOME} )
      coolfluid_add_trial_library_path( $ENV{BLAS_HOME}/lib )
    endif()

    find_library(BLAS_LIBRARIES blas ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
    find_library(BLAS_LIBRARIES blas )

    if( BLAS_LIBRARIES )
      set( CF3_HAVE_BLAS 1 CACHE BOOL "Found BLAS library" )
    else()
      set( CF3_HAVE_BLAS 0 )
    endif()

  endif()

  coolfluid_log_file( "CF3_HAVE_BLAS: [${CF3_HAVE_BLAS}]" )
  if(CF3_HAVE_BLAS)
     coolfluid_log_file( "  BLAS_LIBRARIES:     [${BLAS_LIBRARIES}]" )
  endif()

# LAPACK #########################

  if( NOT CF3_HAVE_LAPACK )

    coolfluid_set_trial_include_path("") # clear include search path
    coolfluid_set_trial_library_path("") # clear library search path


    if( EXISTS ${LAPACK_DIR} )
      coolfluid_add_trial_library_path ( ${LAPACK_DIR}  )
      coolfluid_add_trial_library_path ( ${LAPACK_DIR}/lib )
    endif()

    if( EXISTS $ENV{LAPACK_HOME} )
      coolfluid_add_trial_library_path( $ENV{LAPACK_HOME} )
      coolfluid_add_trial_library_path( $ENV{LAPACK_HOME}/lib )
    endif()

    find_library(LAPACK_LIBRARIES lapack ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
    find_library(LAPACK_LIBRARIES lapack )

  endif()

  if( LAPACK_LIBRARIES )
    set( CF3_HAVE_LAPACK 1 CACHE BOOL "Found LAPACK library")
  else()
    set( CF3_HAVE_LAPACK 0 )
  endif()


  coolfluid_log_file( "CF3_HAVE_LAPACK: [${CF3_HAVE_LAPACK}]" )
  if(CF3_HAVE_LAPACK)
    coolfluid_log_file( "  LAPACK_LIBRARIES:   [${LAPACK_LIBRARIES}]" )
  endif()

# BOTH ###########################

  if( CF3_HAVE_BLAS AND CF3_HAVE_LAPACK )
    set( BLASLAPACK_LIBRARIES   "${LAPACK_LIBRARIES} ${BLAS_LIBRARIES}" CACHE STRING "BLAS and LAPACK libraries")
    set( CF3_HAVE_BLASLAPACK 1 CACHE BOOL "Found BLAS and LAPACK libraries")

    coolfluid_log_file( "CF3_HAVE_BLASLAPACK: [${CF3_HAVE_BLASLAPACK}]" )
    coolfluid_log_file( "  BLASLAPACK_LIBRARIES: [${BLASLAPACK_LIBRARIES}]" )

  endif()

  mark_as_advanced( LAPACK_LIBRARIES BLAS_LIBRARIES )

#################################

else()

  # user provided directly the libraries of LAPACK
  # TODO: test  that they actually work

  set(CF3_HAVE_LAPACK       ON CACHE BOOL "Found LAPACK library")
  set(CF3_HAVE_BLAS         ON CACHE BOOL "Found BLAS   library")
  set(CF3_HAVE_BLASLAPACK   ON CACHE BOOL "Found BLAS and LAPACK libraries")

  set( BLASLAPACK_LIBRARIES   "${LAPACK_LIBRARIES}" CACHE STRING "BLAS and LAPACK libraries")

  mark_as_advanced( BLASLAPACK_LIBRARIES LAPACK_LIBRARIES )

  coolfluid_log_file( "  CF3_HAVE_BLASLAPACK: [${CF3_HAVE_BLASLAPACK}]" )
  coolfluid_log_file( "  BLASLAPACK_LIBRARIES: [${BLASLAPACK_LIBRARIES}]" )

endif()

mark_as_advanced( CF3_HAVE_LAPACK CF3_HAVE_BLAS CF3_HAVE_BLASLAPACK BLASLAPACK_LIBRARIES )

#if ( ${CF3_HAVE_BLASLAPACK} )
#    list( APPEND CF3_DEPS_LIBRARIES ${BLASLAPACK_LIBRARIES} )
#endif()

set( BlasLapack_FOUND ${CF3_HAVE_BLASLAPACK} )
coolfluid_set_package( PACKAGE BlasLapack DESCRIPTION "linear algebra"
                       VARS BLASLAPACK_LIBRARIES)
