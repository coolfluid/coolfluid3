#
# this module look for PARMETIS (http://hdf.ncsa.uiuc.edu) support
# it will define the following values
#
# PARMETIS_INCLUDE_DIR  = where parmetis.h can be found
# PARMETIS_LIBRARY      = the library to link against (parmetis etc)
# CF_HAVE_PARMETIS        = set to true after finding the library
#

OPTION( CF_SKIP_PARMETIS "Skip search for Parmetis library" OFF )

# dont search for parmetis without MPI
if( NOT CF_SKIP_PARMETIS OR NOT CF_HAVE_MPI )

  SET_TRIAL_INCLUDE_PATH ("") # clear include search path
  SET_TRIAL_LIBRARY_PATH ("") # clear library search path

  ADD_TRIAL_INCLUDE_PATH( ${PARMETIS_ROOT}/include )
  ADD_TRIAL_INCLUDE_PATH( $ENV{PARMETIS_ROOT}/include )

  FIND_PATH(PARMETIS_INCLUDE_DIR parmetis.h  ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  FIND_PATH(PARMETIS_INCLUDE_DIR parmetis.h )

  ADD_TRIAL_LIBRARY_PATH(${PARMETIS_ROOT}/lib )
  ADD_TRIAL_LIBRARY_PATH($ENV{PARMETIS_ROOT}/lib )

  FIND_LIBRARY(PARMETIS_LIB_PARMETIS parmetis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  FIND_LIBRARY(PARMETIS_LIB_PARMETIS parmetis )

  FIND_LIBRARY(PARMETIS_LIB_METIS metis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  FIND_LIBRARY(PARMETIS_LIB_METIS metis )

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
