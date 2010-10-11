###############################################################################
# assertions
if( NOT CF_ENABLE_ASSERTIONS )
  add_definitions( -DNDEBUG )
endif()

###############################################################################
# Logging options

if( NOT CF_ENABLE_TRACE)
  add_definitions(-DCF_NO_TRACE)
endif()

if( NOT CF_ENABLE_LOGALL )
  add_definitions(-DCF_NO_LOG)
endif()

if( NOT CF_ENABLE_LOGDEBUG)
  add_definitions(-DCF_NO_DEBUG_LOG)
endif()

if( NOT CF_ENABLE_DEBUG_MACROS)
  add_definitions(-DCF_NO_DEBUG_MACROS)
endif()

###############################################################################
# process precision option
if( CF_USER_PRECISION MATCHES "[Ss][Ii][Nn][Gg][Ll][Ee]" )
  set( CF_REAL_IS_FLOAT ON CACHE STRING "Real type float" FORCE )
endif()

if( CF_USER_PRECISION MATCHES "[Dd][Oo][Uu][Bb][Ll][Ee]" )
  set( CF_REAL_IS_DOUBLE ON CACHE STRING "Real type double" FORCE )
endif()

if( CF_USER_PRECISION MATCHES "[Qq][Uu][Aa][Dd]" )
  set( CF_REAL_IS_LONGDOUBLE ON CACHE STRING "Real type long double" FORCE )
endif()

# default is double precision
if( NOT DEFINED CF_REAL_IS_FLOAT OR NOT DEFINED CF_REAL_IS_DOUBLE )
  set( CF_REAL_IS_DOUBLE ON CACHE STRING "Real type double" FORCE )
endif()

mark_as_advanced( CF_REAL_TYPE )

###############################################################################
# explicit template support

if( CF_ENABLE_EXPLICIT_TEMPLATES AND CF_CXX_SUPPORTS_EXPLICIT_TEMPLATES )
  set( CF_HAVE_CXX_EXPLICIT_TEMPLATES ON CACHE BOOL "Support for Explicit templates activated" )
else()
  set( CF_HAVE_CXX_EXPLICIT_TEMPLATES OFF CACHE BOOL "Support for Explicit templates deactivated" )
endif()

# Apple linker with GCC does not support explicit templates so force them OFF
if( APPLE AND CMAKE_COMPILER_IS_GNUCC )
  set( CF_HAVE_CXX_EXPLICIT_TEMPLATES OFF CACHE BOOL "Support for explicit templates deactivated -- Apple with GCC don't support it" FORCE )
  if( CF_ENABLE_EXPLICIT_TEMPLATES )
    coolfluid_log( "Explicit templates requested but not supported on Mac OS X" )
  endif()
endif()

coolfluid_log_file( "CF_HAVE_CXX_EXPLICIT_TEMPLATES [${CF_HAVE_CXX_EXPLICIT_TEMPLATES}]" )

mark_as_advanced( CF_HAVE_CXX_EXPLICIT_TEMPLATES )

###############################################################################
# sys and time together
if( CF_HAVE_SYS_TIME_H AND CF_HAVE_TIME_H )
  set( CF_TIME_WITH_SYS_TIME 1 CACHE BOOL "Have time.h and sys/time.h together")
  mark_as_advanced( CF_TIME_WITH_SYS_TIME )
endif()

#########################################################################################
# PROFILING OPTIONS
#########################################################################################

if(CF_ENABLE_PROFILING)

  # by default the profiler is google
  if( NOT DEFINED CF_PROFILER_TOOL )
    set( CF_PROFILER_TOOL google-perftools )
  endif()

  ###########################
  # GNU gprof
  if(CF_PROFILER_TOOL MATCHES gprof)
    if(UNIX AND CMAKE_COMPILER_IS_GNUCC)
      set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -pg" )
      set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -pg" )
    else()
      coolfluid_log("User selected profiler [gprof] must be used with GCC compiler")
      set( CF_PROFILER_TOOL     NOTFOUND )
    endif()
  endif()

  ###########################
  # google-perftools
  if(CF_PROFILER_TOOL MATCHES google-perftools)

    # a link library will be added to each executable target
    if( CF_HAVE_GOOGLE_PERFTOOLS )
      set( CF_PROFILER_IS_GOOGLE  ON )
    else()
      coolfluid_log("User selected profiler [google-pertools] could not be found")
      set( CF_PROFILER_GOOGLE  OFF )
      set( CF_PROFILER_IS_GOOGLE    NOTFOUND )
    endif()
    mark_as_advanced(CF_PROFILER_IS_GOOGLE)

  endif()

endif()

#########################################################################################
# STATIC BUILD OPTIONS
#########################################################################################

if( CF_ENABLE_STATIC )

  set(BUILD_SHARED_LIBS OFF)

    if(UNIX)
      # LINUX
      if("${CMAKE_SYSTEM}" MATCHES Linux)
        set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> -Wl,-whole-archive <LINK_LIBRARIES> -Wl,-no-whole-archive")
      endif("${CMAKE_SYSTEM}" MATCHES Linux)
      # SGI IRIX
      if("${CMAKE_SYSTEM}" MATCHES IRIX)
        set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> -Wl,-all <LINK_LIBRARIES> -Wl,-notall")
      endif("${CMAKE_SYSTEM}" MATCHES IRIX)
      # On Darwin:
      #  -all_load $convenience
      if("${CMAKE_SYSTEM}" MATCHES Darwin)
        set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> -all_load <LINK_LIBRARIES>")
      endif("${CMAKE_SYSTEM}" MATCHES Darwin)
      # On Solaris 2:
      #   -z allextract $convenience -z defaultextract
    endif(UNIX)

else()

  set( BUILD_SHARED_LIBS ON )

endif()






