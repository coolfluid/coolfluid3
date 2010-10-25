# always define a build - default is RelWithDebInfo
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# capitalize the build type
string( TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_CAPS )
set(CF_BUILD_TYPE_OK OFF)

#######################################################
### DEBUG mode

if(CMAKE_BUILD_TYPE_CAPS MATCHES "DEBUG")

  set(CF_BUILD_TYPE_OK ON)

  if( CMAKE_COMPILER_IS_GNUCC AND CF_ENABLE_STDDEBUG )
      list( APPEND CF_CXX_FLAGS_DEBUG   " -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC" )
      mark_as_advanced( CF_CXX_FLAGS_DEBUG )
  endif()

  if( WIN32 )
    list( APPEND CF_C_FLAGS_DEBUG       "/Zi" )
    list( APPEND CF_CXX_FLAGS_DEBUG     "/Zi" )
    mark_as_advanced( CF_C_FLAGS_DEBUG  CF_CXX_FLAGS_DEBUG )
  endif()


  coolfluid_set_if_not_defined( CF_ENABLE_ASSERTIONS   ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_TRACE        ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGALL       ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGDEBUG     ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_DEBUG_MACROS ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_STATIC       OFF  )

endif()

#######################################################
### RELWITHDEBINFO mode

if(CMAKE_BUILD_TYPE_CAPS MATCHES "RELWITHDEBINFO")

  set(CF_BUILD_TYPE_OK ON)

  coolfluid_set_if_not_defined( CF_ENABLE_ASSERTIONS   ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_TRACE        ON  )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGALL       ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGOPTIM     ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_OPTIM_MACROS ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_STATIC       OFF  )

endif()

#######################################################
### RELEASE mode

if(CMAKE_BUILD_TYPE_CAPS MATCHES "RELEASE")

  set(CF_BUILD_TYPE_OK ON)

  if(CMAKE_COMPILER_IS_GNUCC)
    list( APPEND CF_C_FLAGS_RELEASE       "-O3 -fomit-frame-pointer" )
    list( APPEND CF_CXX_FLAGS_RELEASE     "-O3 -fomit-frame-pointer" )
    list( APPEND CF_Fortran_FLAGS_RELEASE "-O3 -fomit-frame-pointer" )
    mark_as_advanced( CF_CMAKE_CXX_FLAGS_RELEASE  CF_CMAKE_C_FLAGS_RELEASE CF_CMAKE_Fortran_FLAGS_RELEASE )
  endif()

  coolfluid_set_if_not_defined( CF_ENABLE_ASSERTIONS   OFF  )
  coolfluid_set_if_not_defined( CF_ENABLE_TRACE        OFF  )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGALL       ON   )
  coolfluid_set_if_not_defined( CF_ENABLE_LOGDEBUG     OFF  )
  coolfluid_set_if_not_defined( CF_ENABLE_DEBUG_MACROS OFF  )
  coolfluid_set_if_not_defined( CF_ENABLE_STATIC       OFF  )

endif()

#######################################################

# check that the build type belongs to the list [ Debug Release RelWithDebInfo MinSizeRel ]
if( NOT CF_BUILD_TYPE_OK )
  message( FATAL_ERROR "Build type (case insensitive) [${CMAKE_BUILD_TYPE_CAPS}] is not one of [ Debug Release RelWithDebInfo MinSizeRel ]" )
endif()

#######################################################

# apply additions to flags
if( DEFINED CF_C_FLAGS_${CMAKE_BUILD_TYPE_CAPS} )
  set( CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_CAPS}  "${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_CAPS}} ${CF_C_FLAGS_${CMAKE_BUILD_TYPE_CAPS}}"  CACHE STRING "Flags used by the C compiler during ${CMAKE_BUILD_TYPE} builds." FORCE )
endif()
if( DEFINED CF_CXX_FLAGS_${CMAKE_BUILD_TYPE_CAPS} )
  set( CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_CAPS}  "${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_CAPS}} ${CF_CXX_FLAGS_${CMAKE_BUILD_TYPE_CAPS}}"  CACHE STRING "Flags used by the C++ compiler during ${CMAKE_BUILD_TYPE} builds." FORCE )
endif()
if( DEFINED CF_Fortran_FLAGS_${CMAKE_BUILD_TYPE_CAPS} )
  set( CMAKE_Fortran_FLAGS_${CMAKE_BUILD_TYPE_CAPS}  "${CF_Fortran_FLAGS_${CMAKE_BUILD_TYPE_CAPS}}"  CACHE STRING "Flags used by the C compiler during ${CMAKE_BUILD_TYPE} builds." FORCE )
endif()

