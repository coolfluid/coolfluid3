##############################################################################
# macro for adding a acceptance tests
##############################################################################

function( coolfluid_add_acceptance_test  )

  set( options ) # none
  set( single_value_args NAME SCRIPT )
  set( multi_value_args  CONDITIONS ) # none

  cmake_parse_arguments(_PAR "${options}" "${single_value_args}" "${multi_value_args}"  ${_FIRST_ARG} ${ARGN})

  if(_PAR_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to coolfluid_add_acceptance_test(): \"${_PAR_UNPARSED_ARGUMENTS}\"")
  endif()

  if(NOT _PAR_NAME)
    message(FATAL_ERROR "The call to coolfluid_add_acceptance_test() doesn't set the required NAME argument.")
  endif()
  if(NOT _PAR_SCRIPT)
    message(FATAL_ERROR "The call to coolfluid_add_acceptance_test() doesn't set the required SCRIPT argument.")
  endif()

  # check all conditions

  set( _GATHER_CONDITION TRUE )
  foreach( cond ${_PAR_CONDITIONS} )
    if( _GATHER_CONDITION AND ${cond} )
      set( _GATHER_CONDITION TRUE )
    else()
      set( _GATHER_CONDITION FALSE )
    endif()
  endforeach()

  # add the test

  if( CF_ENABLE_ACCEPTANCE_TESTS AND coolfluid-command_builds AND _GATHER_CONDITION )

    coolfluid_log( " ADDED ATEST [${_PAR_NAME}]" )

    set( TESTDIR ${CMAKE_CURRENT_BINARY_DIR} )

    # put the coolfluid-shell script there

    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${_PAR_SCRIPT} ${TESTDIR}/${_PAR_SCRIPT} @ONLY )

    # Apparently needs cmake 2.8.4 or greater
    #            WORKING_DIRECTORY ${TESTDIR}

    add_test( NAME ${_PAR_NAME}
              COMMAND coolfluid-command -f ${_PAR_SCRIPT} )
  else()
    coolfluid_log( " SKIPPED ATEST [${_PAR_NAME}]" )
  endif()


endfunction( coolfluid_add_acceptance_test  )

##############################################################################
