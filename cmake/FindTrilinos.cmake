# this module looks for the Trilinos library
# it will define the following values
#
# Needs environmental variables
#   TRILINOS_HOME
# Sets
#   TRILINOS_INCLUDE_DIR
#   TRILINOS_LIBRARIES
#   CF3_HAVE_TRILINOS
#
option( CF3_SKIP_TRILINOS "Skip search for Zoltan library" OFF )

coolfluid_set_trial_include_path("") # clear include search path
coolfluid_set_trial_library_path("") # clear library search path

coolfluid_add_trial_include_path( ${TRILINOS_HOME}/include )
coolfluid_add_trial_include_path( $ENV{TRILINOS_HOME}/include )

find_path( TRILINOS_INCLUDE_DIR Epetra_SerialComm.h PATHS ${TRIAL_INCLUDE_PATHS}  NO_DEFAULT_PATH )
find_path( TRILINOS_INCLUDE_DIR Epetra_SerialComm.h )

coolfluid_add_trial_library_path(${TRILINOS_HOME}/lib )
coolfluid_add_trial_library_path($ENV{TRILINOS_HOME}/lib)

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
  list( APPEND TRILINOS_INCLUDE_DIR ${PARMETIS_INCLUDE_DIR} )
endif()

if( ${CF3_HAVE_PTSCOTCH} )
  list( APPEND TRILINOS_LIBRARIES ${PTSCOTCH_LIBRARIES} )
  list( APPEND TRILINOS_INCLUDE_DIR ${PTSCOTCH_INCLUDE_DIR} )
endif()

coolfluid_add_package( PACKAGE Trilinos
                       DESCRIPTION "parallel linear system solver"
                       URL "http://trilinos.sandia.gov"
                       VARS
                       TRILINOS_INCLUDE_DIR TRILINOS_LIBRARIES  )
