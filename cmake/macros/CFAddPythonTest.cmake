##############################################################################
# macro for adding a acceptance tests
##############################################################################

function( coolfluid_add_python_test  )

  if( CF3_ENABLE_PYTHON AND PYTHONLIBS_FOUND AND Boost_PYTHON_FOUND)
    set( options ATEST UTEST MPI ) # valid test type
    set( single_value_args NAME SCRIPT NBPROCS )
    set( multi_value_args ARGUMENTS) # arguments to the python script itself

    cmake_parse_arguments(_PAR "${options}" "${single_value_args}" "${multi_value_args}"  ${_FIRST_ARG} ${ARGN})

    if(_PAR_UNPARSED_ARGUMENTS)
      message(FATAL_ERROR "Unknown keywords given to coolfluid_add_python_test(): \"${_PAR_UNPARSED_ARGUMENTS}\"")
    endif()

    if(NOT (_PAR_ATEST OR _PAR_UTEST))
      message(FATAL_ERROR "The call to coolfluid_add_python_test() doesn't specify the test type.")
    endif()
    if(NOT _PAR_NAME)
      message(FATAL_ERROR "The call to coolfluid_add_python_test() doesn't set the required NAME argument.")
    endif()
    if(NOT _PAR_SCRIPT)
      message(FATAL_ERROR "The call to coolfluid_add_python_test() doesn't set the required SCRIPT argument.")
    endif()
    if(_PAR_MPI AND NOT _PAR_NBPROCS)
      message(FATAL_ERROR "The call to coolfluid_add_python_test() sets the MPI keyword without specifying NBPROCS")
    endif()

    # add the test

    set(SCRIPT_COMMAND ${PYTHON_EXECUTABLE})
    if(_PAR_MPI AND CF3_HAVE_MPI)
      set(SCRIPT_COMMAND ${CF3_MPIRUN_PROGRAM} -np ${_PAR_NBPROCS} ${SCRIPT_COMMAND})
    endif()

    if( (_PAR_ATEST AND CF3_ENABLE_ACCEPTANCE_TESTS) OR (_PAR_UTEST AND CF3_ENABLE_UNIT_TESTS) )

      coolfluid_log_file( " ADDED PYTHON TEST [${_PAR_NAME}]" )

      set( TESTDIR ${CMAKE_CURRENT_BINARY_DIR} )

      add_test(NAME ${_PAR_NAME}
               COMMAND ${SCRIPT_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/${_PAR_SCRIPT} ${_PAR_ARGUMENTS})
      set_tests_properties(${_PAR_NAME} PROPERTIES ENVIRONMENT "PYTHONPATH=${coolfluid_BINARY_DIR}/dso")

    else()
      coolfluid_log_file( " SKIPPED PYTHON TEST [${_PAR_NAME}]" )
    endif()

  endif()

endfunction( coolfluid_add_python_test  )

##############################################################################
