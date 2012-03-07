# always define a build - default is RelWithDebInfo
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# capitalize the build type
string( TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_CAPS )
set(CF3_BUILD_TYPE_OK OFF)

#######################################################
### DEBUG mode

if(CMAKE_BUILD_TYPE_CAPS MATCHES "DEBUG")

  set(CF3_BUILD_TYPE_OK ON)

  if( CMAKE_COMPILER_IS_GNUCC AND CF3_ENABLE_STDDEBUG )
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC" CACHE STRING "")
  endif()

  if( WIN32 )
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi" CACHE STRING "")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi" CACHE STRING "")
  endif()

endif()

#######################################################
### RELWITHDEBINFO mode

if(CMAKE_BUILD_TYPE_CAPS MATCHES "RELWITHDEBINFO")

  set(CF3_BUILD_TYPE_OK ON)

endif()

#######################################################
### RELEASE mode
if(CMAKE_BUILD_TYPE_CAPS MATCHES "RELEASE")
  set(CF3_BUILD_TYPE_OK ON)

  if(CMAKE_COMPILER_IS_GNUCC)

    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
    list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
    list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
    
    # A bug in GCC 4.1 prevents O3 from working with Eigen
    if(GCC_MAJOR EQUAL 4 AND GCC_MINOR EQUAL 1)
      set(CMAKE_C_FLAGS_RELEASE "-O2 -fomit-frame-pointer" CACHE STRING "" FORCE)
      set(CMAKE_CXX_FLAGS_RELEASE "-O2 -fomit-frame-pointer" CACHE STRING "" FORCE)
      set(CMAKE_Fortran_FLAGS_RELEASE "-O2 -fomit-frame-pointer" CACHE STRING "" FORCE)
    else()
      set(CMAKE_C_FLAGS_RELEASE "-O3 -fomit-frame-pointer" CACHE STRING "" FORCE)
      set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fomit-frame-pointer" CACHE STRING "" FORCE)
      set(CMAKE_Fortran_FLAGS_RELEASE "-O3 -fomit-frame-pointer" CACHE STRING "" FORCE)
    endif()

  endif()

  add_definitions(-DNDEBUG -DCF3_NO_DEBUG_MACROS -DBOOST_DISABLE_ASSERTS)

endif()

#######################################################

# check that the build type belongs to the list [ Debug Release RelWithDebInfo MinSizeRel ]
if( NOT CF3_BUILD_TYPE_OK )
  message( FATAL_ERROR "Build type (case insensitive) [${CMAKE_BUILD_TYPE_CAPS}] is not one of [ Debug Release RelWithDebInfo MinSizeRel ]" )
endif()

#######################################################


