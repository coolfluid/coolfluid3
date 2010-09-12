###############################################################################
### Check for Linux
if( UNIX AND NOT APPLE )
  if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    set( CF_OS_LINUX 1 )
  else()
    set( CF_OS_UNRECOGNIZED_REASON "Unrecognized UNIX type : coolfluid has only been tested for Linux or MacOSX type UNIX'es")
  endif()
endif()

###############################################################################
### Check for Apple MacOSX
if( APPLE )
  if( UNIX AND ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    set( CF_OS_MACOSX 1 )
  else()
    set( CF_OS_UNRECOGNIZED_REASON "Unrecognized APPLE type : coolfluid has only been tested  only with Apple MacOSX ( Darwin ) systems.")
  endif()
endif()

###############################################################################
### Check for Windows
if( WIN32 )
  if( MSVC OR MINGW )
    set( CF_OS_WINDOWS 1 )
  else()
    set( CF_OS_UNRECOGNIZED_REASON "Unrecognized WINDOWS type : coolfluid has only been tested with Win32 and MSVC or MingGW compiler.")
  endif()
endif()

###############################################################################
### FINAL MESSAGE
if( CF_OS_UNRECOGNIZED_REASON )
  set( CF_OS_UNRECOGNIZED 1 )
  if( NOT CF_SKIP_OS_TEST )
    set( FULL_MSG "${CF_OS_UNRECOGNIZED_REASON} Set CMake variable CF_SKIP_OS_TEST to avoid this error" )
    message( FATAL_ERROR ${FULL_MSG} )
  else()
    message( STATUS ${CF_OS_UNRECOGNIZED_REASON} )
    message( STATUS "Nevertheless we try to continue ..." )
  endif()
endif()

###############################################################################

