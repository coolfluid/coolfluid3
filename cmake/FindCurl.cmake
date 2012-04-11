#Sets:
# CURL_INCLUDE_DIRS   = where curl.h can be found
# CURL_LIBRARIES      = the library to link against (curl etc)
# CF3_HAVE_CURL       = set to true after finding the library

option( CF3_SKIP_CURL "Skip search for Curl library" OFF )

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${CURL_HOME}/include )
  coolfluid_add_trial_include_path( $ENV{CURL_HOME}/include )

  find_path(CURL_INCLUDE_DIRS curl/curl.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
  find_path(CURL_INCLUDE_DIRS curl/curl.h)

  coolfluid_add_trial_library_path( ${CURL_HOME}/bin ${CURL_HOME}/lib $ENV{CURL_HOME}/bin ${CURL_HOME}/lib )

  find_library(CURL_LIBRARIES curl ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(CURL_LIBRARIES curl )

coolfluid_set_package( PACKAGE Curl
                       DESCRIPTION "URL data access"
                       URL "http://curl.haxx.se"
                       PURPOSE "Runtime downloading"
                       TYPE RUNTIME
                       VARS CURL_INCLUDE_DIRS CURL_LIBRARIES )

