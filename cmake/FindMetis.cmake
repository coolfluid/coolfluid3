#
# this module look for METIS (http://hdf.ncsa.uiuc.edu) support
# it will define the following values
#
# METIS_INCLUDE_DIR  = where metis.h can be found
# METIS_LIBRARY      = the library to link against (metis etc)
# CF_HAVE_METIS        = set to true after finding the library
#

OPTION ( CF_SKIP_METIS "Skip search for Metis library" OFF )
if( NOT CF_SKIP_METIS )

  SET_TRIAL_INCLUDE_PATH ("") # clear include search path
  SET_TRIAL_LIBRARY_PATH ("") # clear library search path

  ADD_TRIAL_INCLUDE_PATH( ${METIS_HOME}/include )
  ADD_TRIAL_INCLUDE_PATH( $ENV{METIS_HOME}/include )

  FIND_PATH(METIS_INCLUDE_DIR metis.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  FIND_PATH(METIS_INCLUDE_DIR metis.h)

  ADD_TRIAL_LIBRARY_PATH(${METIS_HOME}/lib )
  ADD_TRIAL_LIBRARY_PATH($ENV{METIS_HOME}/lib )

  FIND_LIBRARY(METIS_LIBRARY metis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  FIND_LIBRARY(METIS_LIBRARY metis )

  if(METIS_INCLUDE_DIR AND METIS_LIBRARY)
    set(CF_HAVE_METIS 1 CACHE BOOL "Found metis library")
  else()
    set(CF_HAVE_METIS 0 CACHE BOOL "Not fount metis library")
  endif()

else()
    set(CF_HAVE_METIS 0 CACHE BOOL "Skipped METIS library")
endif()

mark_as_advanced(
  METIS_INCLUDE_DIR
  METIS_LIBRARY
  CF_HAVE_METIS
)

coolfluid_log( "CF_HAVE_METIS: [${CF_HAVE_METIS}]" )
if(CF_HAVE_METIS)
  coolfluid_log_file( "  METIS_INCLUDE_DIR: [${METIS_INCLUDE_DIR}]" )
  coolfluid_log_file( "  METIS_LIBRARY: [${METIS_LIBRARY}]" )
endif()
