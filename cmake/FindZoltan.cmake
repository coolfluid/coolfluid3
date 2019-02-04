# this module looks for Zoltan library
# it will define the following values
#
# Needs environmental variables
#   ZOLTAN_HOME
# Sets
#   ZOLTAN_INCLUDE_DIRS
#   ZOLTAN_LIBRARIES
#   CF3_HAVE_ZOLTAN
#
option( CF3_SKIP_ZOLTAN "Skip search for Zoltan library" OFF )

if( NOT CF3_SKIP_ZOLTAN )

    # Try to find Zoltan using Trilinos recommendations
    if( DEFINED TRILINOS_HOME AND NOT DEFINED ZOLTAN_HOME )
        set( ZOLTAN_HOME ${TRILINOS_HOME} )
    endif()

    if( DEFINED Trilinos_DIR AND NOT DEFINED ZOLTAN_HOME )
        set( ZOLTAN_HOME "${Trilinos_DIR}/../Zoltan" )
    endif()

    if( DEFINED ZOLTAN_HOME )
        find_package(Zoltan PATHS ${ZOLTAN_HOME}/lib/cmake/Zoltan ${ZOLTAN_HOME}/include )
    endif()

    if( DEFINED DEPS_ROOT )
        find_package(Zoltan PATHS ${DEPS_ROOT}/lib/cmake/Zoltan ${DEPS_ROOT}/include )
    endif()

    if(Zoltan_FOUND)

      set( ZOLTAN_INCLUDE_DIRS "" )

      list( APPEND ZOLTAN_INCLUDE_DIRS ${Zoltan_INCLUDE_DIRS})
      list( APPEND ZOLTAN_INCLUDE_DIRS ${Zoltan_TPL_INCLUDE_DIRS})

      set( ZOLTAN_LIBRARIES "" )

      foreach( test_lib ${Zoltan_LIBRARIES} )
        find_library( ${test_lib}_lib ${test_lib} PATHS  ${Zoltan_LIBRARY_DIRS}  NO_DEFAULT_PATH)
        find_library( ${test_lib}_lib ${test_lib})
        mark_as_advanced( ${test_lib}_lib )
        list( APPEND ZOLTAN_LIBRARIES ${${test_lib}_lib} )
      endforeach()

      list( APPEND ZOLTAN_LIBRARIES ${Zoltan_TPL_LIBRARIES} )

    endif()

endif( NOT CF3_SKIP_ZOLTAN )

coolfluid_log_file("ZOLTAN_INCLUDE_DIRS = ${ZOLTAN_INCLUDE_DIRS}" )
coolfluid_log_file("ZOLTAN_LIBRARIES = ${ZOLTAN_LIBRARIES}" )

coolfluid_set_package( PACKAGE Zoltan
                       DESCRIPTION "parallel graph partitioning"
                       URL "http://trilinos.sandia.gov/packages/zoltan"
                       TYPE OPTIONAL
                       VARS
                       ZOLTAN_INCLUDE_DIRS
                       ZOLTAN_LIBRARIES
                       QUIET )
