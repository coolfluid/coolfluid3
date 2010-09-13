#
# this module look for PARMETIS (http://hdf.ncsa.uiuc.edu) support
# it will define the following values
#
# PARMETIS_INCLUDE_DIR  = where parmetis.h can be found
# PARMETIS_LIBRARY      = the library to link against (parmetis etc)
# CF_HAVE_PARMETIS        = set to true after finding the library
#

option( CF_SKIP_PARMETIS "Skip search for Parmetis library" OFF )

# dont search for parmetis without MPI
if( NOT CF_SKIP_PARMETIS OR NOT CF_HAVE_MPI )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${PARMETIS_ROOT}/include )
  coolfluid_add_trial_include_path( $ENV{PARMETIS_ROOT}/include )

  find_path(PARMETIS_INCLUDE_DIR parmetis.h  ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(PARMETIS_INCLUDE_DIR parmetis.h )

  coolfluid_add_trial_library_path(${PARMETIS_ROOT}/lib )
  coolfluid_add_trial_library_path($ENV{PARMETIS_ROOT}/lib )

  find_library(PARMETIS_LIB_PARMETIS parmetis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PARMETIS_LIB_PARMETIS parmetis )

  find_library(PARMETIS_LIB_METIS metis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(PARMETIS_LIB_METIS metis )

  set( PARMETIS_LIBRARIES ${PARMETIS_LIB_PARMETIS} ${PARMETIS_LIB_METIS} )

  if(PARMETIS_INCLUDE_DIR AND PARMETIS_LIBRARIES)
    set(CF_HAVE_PARMETIS 1)
  else()
    set(CF_HAVE_PARMETIS 0)
  endif()

else()
    set(CF_HAVE_PARMETIS 0)
endif()

mark_as_advanced(
  PARMETIS_INCLUDE_DIR
  PARMETIS_LIB_PARMETIS
  PARMETIS_LIB_METIS
  PARMETIS_LIBRARIES
  CF_HAVE_PARMETIS
)

coolfluid_log( "CF_HAVE_PARMETIS: [${CF_HAVE_PARMETIS}]" )
if(CF_HAVE_PARMETIS)
  coolfluid_log( "  PARMETIS_INCLUDE_DIR: [${PARMETIS_INCLUDE_DIR}]" )
  coolfluid_log( "  PARMETIS_LIBRARIES:   [${PARMETIS_LIBRARIES}]" )
endif(CF_HAVE_PARMETIS)
