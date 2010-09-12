#Sets:
# CURL_INCLUDE_DIR  = where curl.h can be found
# CURL_LIBRARY      = the library to link against (curl etc)
# CF_HAVE_CURL        = set to true after finding the library

OPTION ( CF_SKIP_CURL "Skip search for Curl library" OFF )

if( NOT CF_SKIP_CURL )

  SET_TRIAL_INCLUDE_PATH ("") # clear include search path
  SET_TRIAL_LIBRARY_PATH ("") # clear library search path

  ADD_TRIAL_INCLUDE_PATH( ${CURL_HOME}/include )
  ADD_TRIAL_INCLUDE_PATH( $ENV{CURL_HOME}/include )

  FIND_PATH(CURL_INCLUDE_DIR curl/curl.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  FIND_PATH(CURL_INCLUDE_DIR curl/curl.h)

  ADD_TRIAL_LIBRARY_PATH( ${CURL_HOME}/bin ${CURL_HOME}/lib $ENV{CURL_HOME}/bin ${CURL_HOME}/lib )

  FIND_LIBRARY(CURL_LIBRARY curl ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  FIND_LIBRARY(CURL_LIBRARY curl )

  if(CURL_INCLUDE_DIR AND CURL_LIBRARY)
    set(CF_HAVE_CURL 1 CACHE BOOL "Found curl library")
  else()
    set(CF_HAVE_CURL 0 CACHE BOOL "Not fount curl library")
  endif()
  
else()
    set(CF_HAVE_CURL 0 CACHE BOOL "Skipped Curl library")
endif()

  mark_as_advanced(
    CURL_INCLUDE_DIR
    CURL_LIBRARY
    CF_HAVE_CURL
  )

  coolfluid_log( "CF_HAVE_CURL: [${CF_HAVE_CURL}]" )
  if(CF_HAVE_CURL)
    coolfluid_log( "  CURL_INCLUDE_DIR:  [${CURL_INCLUDE_DIR}]" )
    coolfluid_log( "  CURL_LIBRARY:      [${CURL_LIBRARY}]" )
  endif(CF_HAVE_CURL)
