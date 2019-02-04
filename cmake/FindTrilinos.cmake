# this module looks for the Trilinos library
# it will define the following values
#
# Needs environmental variables
#   TRILINOS_HOME
# Sets
#   TRILINOS_INCLUDE_DIRS
#   TRILINOS_LIBRARIES
#   CF3_HAVE_TRILINOS
#

option( CF3_SKIP_TRILINOS "Skip search for Trilinos library" OFF )

if( NOT CF3_SKIP_TRILINOS )

    set(CF3_TRILINOS_EXTRA_LIBS "" CACHE STRING "Extra libraries needed to link with Trilinos")

    # Try to find Trilinos using Trilinos recommendations

    if( DEFINED TRILINOS_HOME )
        find_package(Trilinos PATHS ${TRILINOS_HOME}/lib/cmake/Trilinos ${TRILINOS_HOME}/include )

    elseif( DEFINED DEPS_ROOT )
        find_package(Trilinos PATHS ${DEPS_ROOT}/lib/cmake/Trilinos ${DEPS_ROOT}/include )
    else()
        find_package(trilinos PATHS ${CMAKE_PREFIX_PATH})
    endif()

    if( Trilinos_FOUND )

        set( TRILINOS_INCLUDE_DIRS "" )

        list( APPEND TRILINOS_INCLUDE_DIRS ${Trilinos_INCLUDE_DIRS} )
        list( APPEND TRILINOS_INCLUDE_DIRS ${Trilinos_TPL_INCLUDE_DIRS} )

        foreach( test_lib ${Trilinos_LIBRARIES} )
          if(NOT ${test_lib} STREQUAL "pytrilinos")
            find_library( ${test_lib}_lib ${test_lib} PATHS  ${Trilinos_LIBRARY_DIRS}  NO_DEFAULT_PATH)
            find_library( ${test_lib}_lib ${test_lib})
            mark_as_advanced( ${test_lib}_lib )
            list( APPEND TRILINOS_LIBRARIES ${${test_lib}_lib} )
          endif()
        endforeach()

        list( APPEND TRILINOS_LIBRARIES ${Trilinos_TPL_LIBRARIES} )
        list( APPEND TRILINOS_LIBRARIES ${CF3_TRILINOS_EXTRA_LIBS} )

    endif()

endif( NOT CF3_SKIP_TRILINOS )

coolfluid_log_file("TRILINOS_INCLUDE_DIRS = ${TRILINOS_INCLUDE_DIRS}" )
coolfluid_log_file("TRILINOS_LIBRARIES = ${TRILINOS_LIBRARIES}" )

coolfluid_set_package( PACKAGE Trilinos
                       DESCRIPTION "Parallel linear system solver and other libraries"
                       URL "http://trilinos.sandia.gov"
                       VARS TRILINOS_INCLUDE_DIRS TRILINOS_LIBRARIES
                       QUIET )
