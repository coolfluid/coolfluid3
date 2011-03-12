#Sets:
# RT_LIBRARIES      = the library to link against (RT etc)
# CF_HAVE_RT        = set to true after finding the library

option( CF_SKIP_RT "Skip search for RT library" OFF )

if( NOT CF_SKIP_RT )

#  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

#  coolfluid_add_trial_include_path( ${RT_HOME}/include )
#  coolfluid_add_trial_include_path( $ENV{RT_HOME}/include )

#  find_path(RT_INCLUDE_DIR RT/RT.h ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH)
#  find_path(RT_INCLUDE_DIR RT/RT.h)

  if( DEFINED RT_HOME )
    coolfluid_add_trial_library_path( ${RT_HOME}/lib )
  endif()

  find_library(RT_LIBRARIES rt ${TRIAL_LIBRARY_PATHS} NO_DEFAULT_PATH)
  find_library(RT_LIBRARIES rt )

  if( RT_LIBRARIES)
    set(CF_HAVE_RT 1 CACHE BOOL "Found RT library")
  else()
    set(CF_HAVE_RT 0 CACHE BOOL "Not fount RT library")
  endif()

else()
    set(CF_HAVE_RT 0 CACHE BOOL "Skipped RT library")
endif()

mark_as_advanced( RT_LIBRARIES CF_HAVE_RT )

if ( ${CF_HAVE_RT} )
    list( APPEND CF_DEPS_LIBRARIES ${RT_LIBRARIES} )
endif()

coolfluid_log( "CF_HAVE_RT: [${CF_HAVE_RT}]" )
if(CF_HAVE_RT)
    coolfluid_log( "  RT_LIBRARIES:      [${RT_LIBRARIES}]" )
endif()
