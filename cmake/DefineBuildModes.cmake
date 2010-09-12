# set the default build to be RelWithDebInfo
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

#######################################################
### DEBUG mode

if(UNIX)
  if(CMAKE_COMPILER_IS_GNUCC)
    list( APPEND CF_C_FLAGS_DEBUG       " -g -O1" )
    list( APPEND CF_CXX_FLAGS_DEBUG     " -g -O1" )
    if(CF_ENABLE_STDDEBUG)
      list( APPEND CF_CXX_FLAGS_DEBUG   " -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC" )
    endif()
    list( APPEND CF_Fortran_FLAGS_DEBUG " -g -O1" )
  endif()
endif()

### TESTING THIS MODIFICATION
if( WIN32 )
    list( APPEND CF_C_FLAGS_DEBUG       "/Zi" )
    list( APPEND CF_CXX_FLAGS_DEBUG     "/Zi" )
    list( APPEND CF_Fortran_FLAGS_DEBUG "" )
endif()

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
  coolfluid_set_if_not_defined( CF_ENABLE_ASSERTIONS   ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_TRACE        ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGALL       ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGDEBUG     ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_DEBUG_MACROS ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_STATIC       OFF  )
endif()

#######################################################
### OPTIM mode

if(UNIX)
  if(CMAKE_COMPILER_IS_GNUCC)
    list( APPEND CF_C_FLAGS_OPTIM       "-g -O2" )
    list( APPEND CF_CXX_FLAGS_OPTIM     "-g -O2" )
    list( APPEND CF_Fortran_FLAGS_OPTIM "-g -O2" )
  endif(CMAKE_COMPILER_IS_GNUCC)
endif(UNIX)


mark_as_advanced( CF_CMAKE_CXX_FLAGS_OPTIM  CF_CMAKE_C_FLAGS_OPTIM CF_CMAKE_Fortran_FLAGS_OPTIM )

if(CMAKE_BUILD_TYPE MATCHES "[Oo][Pp][Tt][Ii][Mm]")
  coolfluid_set_if_not_defined( CF_ENABLE_ASSERTIONS   ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_TRACE        ON  )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGALL       ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGOPTIM     ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_OPTIM_MACROS ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_STATIC       OFF  )
endif()

#######################################################
### RelWithDebInfo mode

if(UNIX)
  if(CMAKE_COMPILER_IS_GNUCC)
    list( APPEND CF_C_FLAGS_RelWithDebInfo       "-g -O2" )
    list( APPEND CF_CXX_FLAGS_RelWithDebInfo     "-g -O2" )
    list( APPEND CF_Fortran_FLAGS_RelWithDebInfo "-g -O2" )
  endif(CMAKE_COMPILER_IS_GNUCC)
endif(UNIX)


mark_as_advanced( CF_CMAKE_CXX_FLAGS_RelWithDebInfo  CF_CMAKE_C_FLAGS_RelWithDebInfo CF_CMAKE_Fortran_FLAGS_RelWithDebInfo )

if(CMAKE_BUILD_TYPE MATCHES "[Re][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo]")
  coolfluid_set_if_not_defined( CF_ENABLE_ASSERTIONS   ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_TRACE        ON  )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGALL       ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGOPTIM     ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_OPTIM_MACROS ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_STATIC       OFF  )
endif()

#######################################################
### RELEASE mode

if(UNIX)
  if(CMAKE_COMPILER_IS_GNUCC)
    list( APPEND CF_C_FLAGS_RELEASE       "-O3 -fomit-frame-pointer" )
    list( APPEND CF_CXX_FLAGS_RELEASE     "-O3 -fomit-frame-pointer" )
    list( APPEND CF_Fortran_FLAGS_RELEASE "-O3 -fomit-frame-pointer" )
  endif(CMAKE_COMPILER_IS_GNUCC)
endif(UNIX)


mark_as_advanced( CF_CMAKE_CXX_FLAGS_RELEASE  CF_CMAKE_C_FLAGS_RELEASE CF_CMAKE_Fortran_FLAGS_RELEASE )

if(CMAKE_BUILD_TYPE MATCHES "[Re][Ee][Ll][Ee][Aa][Ss][Ee]")
  coolfluid_set_if_not_defined( CF_ENABLE_ASSERTIONS   OFF  )
  coolfluid_set_if_not_defined( CF_ENABLE_TRACE        OFF  )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGALL       ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGDEBUG     OFF  )
  coolfluid_set_if_not_defined( CF_ENABLE_DEBUG_MACROS OFF  )
  coolfluid_set_if_not_defined( CF_ENABLE_STATIC       OFF  )
endif()

#######################################################

if( DEFINED CF_C_FLAGS_${CMAKE_BUILD_TYPE} )
	set( CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}  "${CF_C_FLAGS_${CMAKE_BUILD_TYPE}}"  CACHE STRING "Flags used by the C compiler during ${CMAKE_BUILD_TYPE} builds." FORCE )
endif()
if( DEFINED CF_CXX_FLAGS_${CMAKE_BUILD_TYPE} )
	set( CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}  "${CF_CXX_FLAGS_${CMAKE_BUILD_TYPE}}"  CACHE STRING "Flags used by the C++ compiler during ${CMAKE_BUILD_TYPE} builds." FORCE )
endif()
if( DEFINED CF_Fortran_FLAGS_${CMAKE_BUILD_TYPE} )
	set( CMAKE_Fortran_FLAGS_${CMAKE_BUILD_TYPE}  "${CF_Fortran_FLAGS_${CMAKE_BUILD_TYPE}}"  CACHE STRING "Flags used by the C compiler during ${CMAKE_BUILD_TYPE} builds." FORCE )
endif()

