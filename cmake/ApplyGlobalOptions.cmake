###############################################################################
# assertions
if ( NOT CF_ENABLE_ASSERTIONS )
  add_definitions ( -DNDEBUG )
endif ()

###############################################################################
# Logging options
IF( NOT CF_ENABLE_TRACE)
  ADD_DEFINITIONS(-DCF_NO_TRACE)
ENDIF()

IF( NOT CF_ENABLE_LOGALL )
  ADD_DEFINITIONS(-DCF_NO_LOG)
ENDIF()

IF( NOT CF_ENABLE_LOGDEBUG)
  ADD_DEFINITIONS(-DCF_NO_DEBUG_LOG)
ENDIF()

IF( NOT CF_ENABLE_DEBUG_MACROS)
  ADD_DEFINITIONS(-DCF_NO_DEBUG_MACROS)
ENDIF()

###############################################################################
# if user disables MPI we overwrite the CF_HAVE_MPI variable
IF ( CF_ENABLE_MPI )
  IF ( CF_MPI_AVAILABLE )
    SET ( CF_HAVE_MPI 1 CACHE BOOL "User enabled MPI [FOUND]" )
  ELSE()
    SET ( CF_HAVE_MPI 0 CACHE BOOL "User enabled MPI [NOT-FOUND]" )
  ENDIF()
ELSE()
  SET ( CF_HAVE_MPI 0 CACHE BOOL "User disabled MPI" )
ENDIF ()

###############################################################################
# explicit template support
IF ( CF_ENABLE_EXPLICIT_TEMPLATES AND CF_CXX_SUPPORTS_EXPLICIT_TEMPLATES )
  SET ( CF_HAVE_CXX_EXPLICIT_TEMPLATES ON CACHE BOOL "Support for Explicit templates activated" )
ELSE ()
  SET ( CF_HAVE_CXX_EXPLICIT_TEMPLATES OFF CACHE BOOL "Support for Explicit templates deactivated" )
ENDIF()

###############################################################################
# sys and time together
IF (CF_HAVE_SYS_TIME_H AND CF_HAVE_TIME_H)
  SET ( CF_TIME_WITH_SYS_TIME 1 CACHE BOOL "Have time.h and sys/time.h together")
ENDIF (CF_HAVE_SYS_TIME_H AND CF_HAVE_TIME_H)

#########################################################################################
# PROFILING OPTIONS
#########################################################################################

IF(CF_ENABLE_PROFILING)

  ###########################
  # GNU gprof
  IF(CF_PROFILER_TOOL MATCHES gprof)
    IF(UNIX AND CMAKE_COMPILER_IS_GNUCC)
      SET(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -pg" )
      SET(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -pg" )
    ELSE(UNIX AND CMAKE_COMPILER_IS_GNUCC)
      LOG("User selected profiler [gprof] must be used with GCC compiler")
      SET( CF_PROFILER_TOOL     NOTFOUND )
    ENDIF()
  ENDIF()

  ###########################
  # google-perftools
  IF(CF_PROFILER_TOOL MATCHES google-perftools)

    FIND_PACKAGE(GooglePerftools)

    IF(CF_HAVE_GOOGLE_PERFTOOLS)
      LINK_LIBRARIES(${GOOGLE_PERFTOOLS_LIBRARIES})
    ELSE(CF_HAVE_GOOGLE_PERFTOOLS)
      LOG("User selected profiler [google-pertools] could not be found")
      SET( CF_PROFILER_TOOL     NOTFOUND )
    ENDIF()

  ENDIF()

ENDIF()

#########################################################################################
# STATIC BUILD OPTIONS
#########################################################################################

IF ( CF_ENABLE_STATIC )

  SET(BUILD_SHARED_LIBS OFF)

    IF(UNIX)
      # LINUX
      IF("${CMAKE_SYSTEM}" MATCHES Linux)
        SET(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> -Wl,-whole-archive <LINK_LIBRARIES> -Wl,-no-whole-archive")
      ENDIF("${CMAKE_SYSTEM}" MATCHES Linux)
      # SGI IRIX
      IF("${CMAKE_SYSTEM}" MATCHES IRIX)
        SET(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> -Wl,-all <LINK_LIBRARIES> -Wl,-notall")
      ENDIF("${CMAKE_SYSTEM}" MATCHES IRIX)
      # On Darwin:
      #  -all_load $convenience
      IF("${CMAKE_SYSTEM}" MATCHES Darwin)
        SET(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> -all_load <LINK_LIBRARIES>")
      ENDIF("${CMAKE_SYSTEM}" MATCHES Darwin)
      # On Solaris 2:
      #   -z allextract $convenience -z defaultextract
    ENDIF(UNIX)

ELSE()

  SET( BUILD_SHARED_LIBS ON )

ENDIF()




