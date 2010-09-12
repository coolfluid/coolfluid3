#Sets:
# CURL_INCLUDE_DIR  = where curl.h can be found
# CURL_LIBRARY      = the library to link against (curl etc)
# CF_HAVE_CURL        = set to true after finding the library

option( CF_SKIP_CURL "Skip search for Curl library" OFF )

if( NOT CF_SKIP_CURL )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${CURL_HOME}/include )
  coolfluid_add_trial_include_path( $ENV{CURL_HOME}/include )

  find_path(CURL_INCLUDE_DIR curl/curl.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(CURL_INCLUDE_DIR curl/curl.h)

  coolfluid_coolfluid_add_trial_library_path( ${CURL_HOME}/bin ${CURL_HOME}/lib $ENV{CURL_HOME}/bin ${CURL_HOME}/lib )

  find_library(CURL_LIBRARY curl ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(CURL_LIBRARY curl )

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
