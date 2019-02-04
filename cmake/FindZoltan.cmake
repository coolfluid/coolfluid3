# this module looks for Zoltan library
# it will define the following values
#
# Sets
#   ZOLTAN_INCLUDE_DIRS
#   ZOLTAN_LIBRARIES
#   CF3_HAVE_ZOLTAN
#
option( CF3_SKIP_ZOLTAN "Skip search for Zoltan library" OFF )

if( NOT CF3_SKIP_ZOLTAN )

    find_package(Zoltan PATHS ${Trilinos_DIR}/../Zoltan)

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
