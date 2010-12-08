#
# this module look for METIS (http://hdf.ncsa.uiuc.edu) support
# it will define the following values
#
# METIS_INCLUDE_DIR  = where metis.h can be found
# METIS_LIBRARIES      = the library to link against (metis etc)
# CF_HAVE_METIS        = set to true after finding the library
#

option( CF_SKIP_METIS "Skip search for Metis library" OFF )
if( NOT CF_SKIP_METIS )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${METIS_HOME}/include )
  coolfluid_add_trial_include_path( $ENV{METIS_HOME}/include )

  find_path(METIS_INCLUDE_DIR metis.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(METIS_INCLUDE_DIR metis.h)

  coolfluid_add_trial_library_path(${METIS_HOME}/lib )
  coolfluid_add_trial_library_path($ENV{METIS_HOME}/lib )

  find_library(METIS_LIBRARIES metis ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(METIS_LIBRARIES metis )

  if(METIS_INCLUDE_DIR AND METIS_LIBRARIES)
    set(CF_HAVE_METIS 1 CACHE BOOL "Found metis library")
  else()
    set(CF_HAVE_METIS 0 CACHE BOOL "Not fount metis library")
  endif()

else()
    set(CF_HAVE_METIS 0 CACHE BOOL "Skipped METIS library")
endif()

mark_as_advanced(
  METIS_INCLUDE_DIR
  METIS_LIBRARIES
  CF_HAVE_METIS
)

if ( ${CF_HAVE_METIS} )
    list( APPEND CF_TP_LIBRARIES ${METIS_LIBRARIES} )
endif()

coolfluid_log( "CF_HAVE_METIS: [${CF_HAVE_METIS}]" )
if(CF_HAVE_METIS)
  coolfluid_log_file( "  METIS_INCLUDE_DIR: [${METIS_INCLUDE_DIR}]" )
  coolfluid_log_file( "  METIS_LIBRARIES: [${METIS_LIBRARIES}]" )
endif()
