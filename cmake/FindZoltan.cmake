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


# Try to find Zoltan using Trilinos recommendations
find_package(Zoltan PATHS ${ZOLTAN_HOME}/lib/cmake/Zoltan ${DEPS_ROOT}/lib/cmake/Zoltan)
if(Zoltan_FOUND)
  list( APPEND ZOLTAN_INCLUDE_DIRS ${Zoltan_INCLUDE_DIRS})
  list( APPEND ZOLTAN_INCLUDE_DIRS ${Zoltan_TPL_INCLUDE_DIRS})

  foreach (test_lib ${Zoltan_LIBRARIES})
    find_library( ${test_lib}_lib ${test_lib} PATHS  ${Zoltan_LIBRARY_DIRS}  NO_DEFAULT_PATH)
    find_library( ${test_lib}_lib ${test_lib})
    list( APPEND ZOLTAN_LIBRARIES ${${test_lib}_lib} )
  endforeach()

  list( APPEND ZOLTAN_LIBRARIES ${Zoltan_TPL_LIBRARIES} )

else()

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${ZOLTAN_HOME}/include )
  coolfluid_add_trial_include_path( $ENV{ZOLTAN_HOME}/include )

  coolfluid_add_trial_library_path(${ZOLTAN_HOME}/lib )
  coolfluid_add_trial_library_path($ENV{ZOLTAN_HOME}/lib)


  # If Zoltan installation happened through CMake and Trilinos, a ZoltanConfig.cmake file exists
  # that has information of which third party libraries Zoltan is built with
  find_file( ZOLTAN_CONFIG_FILE ZoltanConfig.cmake ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
  find_file( ZOLTAN_CONFIG_FILE ZoltanConfig.cmake )

  if( ${ZOLTAN_CONFIG_FILE} )

    include( ${ZOLTAN_CONFIG_FILE} )

    list( APPEND ZOLTAN_INCLUDE_DIRS ${Zoltan_INCLUDE_DIRS})
    list( APPEND ZOLTAN_INCLUDE_DIRS ${Zoltan_TPL_INCLUDE_DIRS})

    foreach (test_lib ${Zoltan_LIBRARIES})
      find_library( ${test_lib}_lib ${test_lib} PATHS  ${Zoltan_LIBRARY_DIRS}  NO_DEFAULT_PATH)
      find_library( ${test_lib}_lib ${test_lib})
      list( APPEND ZOLTAN_LIBRARIES ${${test_lib}_lib} )
    endforeach()

    list( APPEND ZOLTAN_LIBRARIES ${Zoltan_TPL_LIBRARIES} )

    coolfluid_log("ZOLTAN_LIBRARIES = ${ZOLTAN_LIBRARIES}")

  # Try to find Zoltan the standard way
  else()

    find_path( ZOLTAN_INCLUDE_DIRS zoltan.h PATHS ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
    find_path( ZOLTAN_INCLUDE_DIRS zoltan.h )

    find_library(ZOLTAN_LIBRARIES zoltan  PATHS  ${TRIAL_LIBRARY_PATHS}  NO_DEFAULT_PATH)
    find_library(ZOLTAN_LIBRARIES zoltan )

    if( ${CF3_HAVE_PARMETIS} )
      list( APPEND ZOLTAN_LIBRARIES ${PARMETIS_LIBRARIES} )
      list( APPEND ZOLTAN_INCLUDE_DIRS ${PARMETIS_INCLUDE_DIRS} )
    endif()

    if( ${CF3_HAVE_PTSCOTCH} )
      list( APPEND ZOLTAN_LIBRARIES ${PTSCOTCH_LIBRARIES} )
      list( APPEND ZOLTAN_INCLUDE_DIRS ${PTSCOTCH_INCLUDE_DIRS} )
    endif()

  endif()

endif()

coolfluid_add_package( PACKAGE Zoltan
                       DESCRIPTION "parallel graph partitioning"
                       URL "http://trilinos.sandia.gov/packages/zoltan"
                       VARS
                       ZOLTAN_INCLUDE_DIRS
                       ZOLTAN_LIBRARIES  )
