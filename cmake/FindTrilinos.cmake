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

# Try to find Trilinos using Trilinos recommendations
find_package(Trilinos PATHS ${TRILINOS_HOME}/lib/cmake/Trilinos ${TRILINOS_HOME}/include ${DEPS_ROOT}/lib/cmake/Trilinos ${DEPS_ROOT}/include)
if(Trilinos_FOUND)

    list( APPEND TRILINOS_INCLUDE_DIRS ${Trilinos_INCLUDE_DIRS})
    list( APPEND TRILINOS_INCLUDE_DIRS ${Trilinos_TPL_INCLUDE_DIRS})

    foreach (test_lib ${Trilinos_LIBRARIES})
      find_library( ${test_lib}_lib ${test_lib} PATHS  ${Trilinos_LIBRARY_DIRS}  NO_DEFAULT_PATH)
      find_library( ${test_lib}_lib ${test_lib})
      list( APPEND TRILINOS_LIBRARIES ${${test_lib}_lib} )
    endforeach()

    list( APPEND TRILINOS_LIBRARIES ${Trilinos_TPL_LIBRARIES} )

else()

  # Try to find Trilinos the hard way

  coolfluid_set_trial_include_path("") # clear include search path
  coolfluid_set_trial_library_path("") # clear library search path

  coolfluid_add_trial_include_path( ${TRILINOS_HOME}/include )
  coolfluid_add_trial_include_path( $ENV{TRILINOS_HOME}/include )

  coolfluid_add_trial_library_path(${TRILINOS_HOME}/lib )
  coolfluid_add_trial_library_path($ENV{TRILINOS_HOME}/lib)

  find_path( TRILINOS_INCLUDE_DIRS Epetra_SerialComm.h PATHS ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
  find_path( TRILINOS_INCLUDE_DIRS Epetra_SerialComm.h )

  list( APPEND trilinos_req_libs
      epetra
      teuchos
      stratimikosamesos
      stratimikosaztecoo
      stratimikosbelos
      stratimikosifpack
      stratimikosml
      stratimikos
      aztecoo
      ml
      belos
      ifpack
      thyra
      thyraepetra
      thyracore
  )

  foreach (test_lib ${trilinos_req_libs})
    find_library( ${test_lib}_lib ${test_lib} PATHS  ${TRIAL_LIBRARY_PATHS}  NO_DEFAULT_PATH)
    find_library( ${test_lib}_lib ${test_lib})
    list( APPEND TRILINOS_LIBRARIES ${${test_lib}_lib} )
  endforeach()

  if( ${CF3_HAVE_PARMETIS} )
    list( APPEND TRILINOS_LIBRARIES ${PARMETIS_LIBRARIES} )
    list( APPEND TRILINOS_INCLUDE_DIRS ${PARMETIS_INCLUDE_DIRS} )
  endif()

  if( ${CF3_HAVE_PTSCOTCH} )
    list( APPEND TRILINOS_LIBRARIES ${PTSCOTCH_LIBRARIES} )
    list( APPEND TRILINOS_INCLUDE_DIRS ${PTSCOTCH_INCLUDE_DIRS} )
  endif()

endif()

#coolfluid_log("TRILINOS_INCLUDE_DIRS = ${TRILINOS_INCLUDE_DIRS}" )
#coolfluid_log("TRILINOS_LIBRARIES = ${TRILINOS_LIBRARIES}" )

coolfluid_set_package( PACKAGE Trilinos
                       DESCRIPTION "parallel linear system solver and other libraries"
                       URL "http://trilinos.sandia.gov"
                       VARS
                       TRILINOS_INCLUDE_DIRS
                       TRILINOS_LIBRARIES  )
