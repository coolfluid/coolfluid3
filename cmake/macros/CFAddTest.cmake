# Function to add a test.
#
# Mandatory keywords:
# - UTEST/ATEST/PTEST
#      choose according to the test profile: unit-test, acceptance-test, performance-test)
#      give as value the name of the test
# - CPP/PYTHON/CFSCRIPT
#      choose according to the test type: c++ code, python script, cfscript
#      give as value the list of source files or script
#
# Optional keywords:
# - ARGUMENTS
#      a list of arguments to pass to the the test execution (default empty)
# - PLUGINS
#      a list of required plugins for the test (default empty)
# - LIBS
#      a list of required libraries for this test to build (default empty)
# - MPI (default: mpirun is not called , unless global CMake constant makes all utests run with mpirun -np 1)
#      number of processors to use for call with mpirun
#      default: mpirun is not called, unless CF3_ALL_UTESTS_PARALLEL is ON, then default=1
# - CONDITION
#      boolean expression to add extra condition if the test should build (default: true)
# - SCALING
#      option to indicate mpi-scaling is used (advanced, should not be used much)
# - MOC
#      list of QT moc files to be included
# - DEPENDS
#      list of targets this test depends on (LIBS are automatically a dependency already)
#
# After calling this function, the test is added to one of the following lists:
#   - CF3_ENABLED_UTESTS
#   - CF3_DISABLED_UTESTS
#   - CF3_ENABLED_ATESTS
#   - CF3_DISABLED_ATESTS
#   - CF3_ENABLED_PTESTS
#   - CF3_DISABLED_PTESTS
# The following variables will be set:
#   - ${TEST_NAME}_builds
#   - ${TEST_NAME}_dir
#   - ${TEST_NAME}_libs

function( coolfluid_add_test )

  set( options SCALING)
  set( single_value_args UTEST ATEST PTEST)
  set( multi_value_args  CPP PYTHON CFSCRIPT ARGUMENTS CONDITION MPI LIBS PLUGINS MOC DEPENDS)

  cmake_parse_arguments(_PAR "${options}" "${single_value_args}" "${multi_value_args}"  ${_FIRST_ARG} ${ARGN})

  # check if all mandatory arguments are given

  foreach(unparsed_arg ${_PAR_UNPARSED_ARGUMENTS})
    if( NOT ( (${unparsed_arg} STREQUAL "AND") OR (${unparsed_arg} STREQUAL "OR") OR (${unparsed_arg} STREQUAL "NOT") ) )
      list( APPEND _UNPARSED_ARGUMENTS ${unparsed_arg} )
    endif()
  endforeach()
  if(_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to coolfluid_add_test(): \"${_UNPARSED_ARGUMENTS}\"")
  endif()
  if( (NOT _PAR_UTEST) AND (NOT _PAR_ATEST) AND (NOT _PAR_PTEST))
    message(FATAL_ERROR "The call to coolfluid_add_test() doesn't set the required \"UTEST/ATEST/PTEST test-name\" argument.")
  endif()
  if( (NOT _PAR_CPP) AND (NOT _PAR_PYTHON) AND (NOT _PAR_CFSCRIPT) )
    message(FATAL_ERROR "The call to coolfluid_add_test() doesn't set the required \"CPP/PYTHON/CFSCRIPT files\" argument.")
  endif()

  if(_PAR_UTEST)
    set(_TEST_NAME ${_PAR_UTEST})
    set(_TEST_PROFILE "unit-test")
    set(_TEST_PROFILE_ENABLED ${CF3_ENABLE_UNIT_TESTS})
  elseif(_PAR_ATEST)
    set(_TEST_NAME ${_PAR_ATEST})
    set(_TEST_PROFILE "acceptance-test")
    set(_TEST_PROFILE_ENABLED ${CF3_ENABLE_ACCEPTANCE_TESTS})
  elseif(_PAR_PTEST)
    set(_TEST_NAME ${_PAR_PTEST})
    set(_TEST_PROFILE "performance-test")
    set(_TEST_PROFILE_ENABLED ${CF3_ENABLE_PERFORMANCE_TESTS})
  endif()

  if(_PAR_CPP)
    set(_TEST_FILES ${_PAR_CPP})
  elseif(_PAR_PYTHON)
    set(_TEST_FILES ${_PAR_PYTHON})
  elseif(_PAR_CFSCRIPT)
    set(_TEST_FILES ${_PAR_CFSCRIPT})
  endif()

  # option to build it or not (option is advanced and does not appear in the cmake gui)
  option( CF3_BUILD_${_TEST_NAME} "Build the ${_TEST_PROFILE} [${_TEST_NAME}]" YES )
  mark_as_advanced(CF3_BUILD_${_TEST_NAME})

  set( _TEST_DIR ${CMAKE_CURRENT_BINARY_DIR} )

  set(_CONDITION_FILE "${CMAKE_CURRENT_BINARY_DIR}/set_${_TEST_NAME}_condition.cmake")

  # check for condition
  if( DEFINED _PAR_CONDITION )
    FILE( WRITE  ${_CONDITION_FILE} "if( ")
    foreach( expression_term ${_PAR_CONDITION} )
      FILE( APPEND ${_CONDITION_FILE} " ${expression_term}")
    endforeach()
    FILE( APPEND ${_CONDITION_FILE} " )\n")
    FILE( APPEND ${_CONDITION_FILE} "  set(_CONDITION TRUE)\n")
    FILE( APPEND ${_CONDITION_FILE} "endif()")
    INCLUDE( ${_CONDITION_FILE} )
  else() # default is to include
    set( _CONDITION TRUE )
  endif()

  # check if mpirun needs to be called

  set( _RUN_MPI ${CF3_ALL_UTESTS_PARALLEL} )
  set( _MPI_NB_PROCS 1 )
  if( _PAR_MPI )
    set( _RUN_MPI ON )
    set( _MPI_NB_PROCS ${_PAR_MPI} )
  endif()

  # check if all required plugins are available

  set( _PLUGINS_AVAILABLE TRUE )
  foreach( plugin ${_PAR_PLUGINS} )
    list( FIND CF3_PLUGIN_LIST ${plugin} pos )
    if( ${pos} EQUAL -1 )
      coolfluid_log_verbose( "     \# ${_TEST_PROFILE} [${_TEST_NAME}] requires plugin [${plugin}] which is not present")
      list(APPEND _UNAVAILABLE_PLUGINS ${plugin})
    endif()
  endforeach()
  if(DEFINED _UNAVALABLE_PLUGINS)
    set( _PLUGINS_AVAILABLE FALSE )
  endif()

  # check if QT is required/available
  set(_TEST_NEEDS_QT_BUT_UNAVAILABLE FALSE)
  if( DEFINED _PAR_MOC )
    if( QT4_FOUND )
      QT4_WRAP_CPP(_MOC ${_PAR_MOC})
    else()
      set(_TEST_NEEDS_QT_BUT_UNAVAILABLE TRUE)
    endif()
  endif()

  # check if scaling test will be created

  set( _TEST_SCALING OFF)

  # separate the source files and remove them from the orphan list

  coolfluid_separate_sources("${_TEST_FILES}" ${_TEST_NAME})

  source_group( Headers FILES ${${_TEST_NAME}_headers} )
  source_group( Sources FILES ${${_TEST_NAME}_sources} )

  # check if test will build

  set(_TEST_BUILDS ${CF3_BUILD_${_TEST_NAME}})
  if( NOT _TEST_PROFILE_ENABLED )
    set(_TEST_BUILDS FALSE)
  endif()
  if( _RUN_MPI AND (NOT CF3_MPI_TESTS_RUN) )
    set(_TEST_BUILDS FALSE)
  endif()
  if( NOT _PLUGINS_AVAILABLE )
    set(_TEST_BUILDS FALSE)
  endif()
  if( NOT _CONDITION )
    set(_TEST_BUILDS FALSE)
  endif()
  if( _TEST_NEEDS_QT_BUT_UNAVAILABLE )
    set(_TEST_BUILDS FALSE)
  endif()


  if( _TEST_BUILDS )

    if( _PAR_CPP )

      if( DEFINED ${_TEST_NAME}_includedirs )
        include_directories(${${_TEST_NAME}_includedirs})
      endif()

      if( DEFINED _MOC)
        add_executable( ${_TEST_NAME} ${${_TEST_NAME}_sources} ${${_TEST_NAME}_headers}  ${_MOC})
      else()
        add_executable( ${_TEST_NAME} ${${_TEST_NAME}_sources} ${${_TEST_NAME}_headers})
      endif()

      if( DEFINED _PAR_DEPENDS)
        add_dependencies( ${_TEST_NAME} ${_PAR_DEPENDS} )
      endif()

      # if mpi was found add it to the libraries
      if(CF3_HAVE_MPI AND NOT CF3_HAVE_MPI_COMPILER)
        target_link_libraries( ${_TEST_NAME} ${MPI_LIBRARIES} )
      endif()

      # add dependency libraries if defined
      if( DEFINED _PAR_LIBS )
        target_link_libraries( ${_TEST_NAME} ${_PAR_LIBS} )
      else()
        target_link_libraries( ${_TEST_NAME} "coolfluid_common" )
      endif()

      # add boost unit test lib
      target_link_libraries( ${_TEST_NAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )

      set(_TEST_COMMAND ${_TEST_NAME})
      if(_RUN_MPI)
        set(_TEST_COMMAND ${CF3_MPIRUN_PROGRAM} -np ${_MPI_NB_PROCS} ${_TEST_COMMAND})
      endif()
      add_test( ${_TEST_NAME} ${_TEST_COMMAND} ${_PAR_ARGUMENTS} )

      if(_TEST_SCALING)
        add_test("${_TEST_NAME}-scaling" ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/test-mpi-scalability.py ${CF3_MPIRUN_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${_TEST_NAME} ${CF3_MPI_TESTS_MAX_NB_PROCS} ${_PAR_ARGUMENTS})
      endif()

    endif( _PAR_CPP )


    if( _PAR_PYTHON AND CF3_HAVE_PYTHON )
      set(SCRIPT_COMMAND ${PYTHON_EXECUTABLE})
      if(_RUN_MPI AND CF3_HAVE_MPI)
        set(SCRIPT_COMMAND ${CF3_MPIRUN_PROGRAM} -np ${_MPI_NB_PROCS} ${SCRIPT_COMMAND})
      endif()
      add_custom_target(${_TEST_NAME} SOURCES ${${_TEST_NAME}_headers} ${${_TEST_NAME}_sources})
      add_test(NAME ${_TEST_NAME}
               COMMAND ${SCRIPT_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/${_TEST_FILES} ${_PAR_ARGUMENTS})
      set_tests_properties(${_TEST_NAME} PROPERTIES ENVIRONMENT "PYTHONPATH=${coolfluid_BINARY_DIR}/dso")

      if(_TEST_SCALING)
        coolfluid_log("Scaling requested for python test. Not implemented yet in build system.")
        set(_TEST_SCALING OFF)
      endif()

    endif( _PAR_PYTHON AND CF3_HAVE_PYTHON )

    if( _PAR_CFSCRIPT )

      set(_TEST_SCRIPT ${_TEST_FILES})
      configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${_TEST_SCRIPT} ${_TEST_DIR}/${_TEST_SCRIPT} @ONLY )

      # Apparently needs cmake 2.8.4 or greater
      #            WORKING_DIRECTORY ${TESTDIR}
      add_custom_target(${_TEST_NAME}
                        SOURCES ${${_TEST_NAME}_headers} ${${_TEST_NAME}_sources}
                        DEPENDS coolfluid-command)
      add_test( NAME ${_TEST_NAME}
                COMMAND coolfluid-command -f ${_TEST_SCRIPT} )

      if(_TEST_SCALING)
        coolfluid_log("Scaling requested for python test. Not implemented yet in build system.")
        set(_TEST_SCALING OFF)
      endif()

    endif( _PAR_CFSCRIPT )
  endif( _TEST_BUILDS )

  if(CF3_INSTALL_TESTS)  # add installation paths
    install( TARGETS ${_TEST_NAME}
             RUNTIME DESTINATION ${CF3_INSTALL_BIN_DIR}
             LIBRARY DESTINATION ${CF3_INSTALL_LIB_DIR}
             ARCHIVE DESTINATION ${CF3_INSTALL_LIB_DIR} )
  endif(CF3_INSTALL_TESTS)

  # Set global variables
  set( ${_TEST_NAME}_builds  ${_TEST_BUILDS} )
  set( ${_TEST_NAME}_dir     ${_TEST_DIR} )
  set( ${_TEST_NAME}_libs    ${_PAR_LIBS} )
  if( DEFINED ${_TEST_NAME}_libs )
    list(REMOVE_DUPLICATES ${_TEST_NAME}_libs)
  endif()

  get_target_property( ${_TEST_NAME}_P_SOURCES   ${_TEST_NAME} SOURCES )
  get_target_property( ${_TEST_NAME}_LINK_FLAGS  ${_TEST_NAME} LINK_FLAGS )
  get_target_property( ${_TEST_NAME}_TYPE        ${_TEST_NAME} TYPE )

  # Log summary
  coolfluid_log_file("${_TEST_PROFILE} ${_TEST_NAME}")
  coolfluid_log_file("    build requested     : [${CF3_BUILD_${_TEST_NAME}}]")
  coolfluid_log_file("    builds              : [${${_TEST_NAME}_builds}]")
  coolfluid_log_file("    sources             : [${_TEST_FILES}]")
  coolfluid_log_file("    test dir            : [${_TEST_DIR}]")
  coolfluid_log_file("    mpirun              : [${_RUN_MPI}]")
  coolfluid_log_file("    mpi nb_proc         : [${_MPI_NB_PROCS}]")
  coolfluid_log_file("    scaling             : [${_TEST_SCALING}]")
  coolfluid_log_file("    required plugins    : [${_PAR_PLUGINS}]")
  coolfluid_log_file("    unavailable plugins : [${_UNAVAILABLE_PLUGINS}]")
  coolfluid_log_file("    libs                : [${${_TEST_NAME}_libs}]")
  coolfluid_log_file("    condition           : [${_CONDITION}]")
  coolfluid_log_file("    target type         : [${${_TEST_NAME}_TYPE}]")
  coolfluid_log_file("    target sources      : [${${_TEST_NAME}_P_SOURCES}]")
  coolfluid_log_file("    target link flags   : [${${_TEST_NAME}_LINK_FLAGS}]")

  # Add test to either CF3_ENABLED_UTESTS  / CF3_ENABLED_ATESTS
  #                 or CF3_DISABLED_UTESTS / CF3_DISABLED_ATESTS

  if(_TEST_BUILDS)
    if(_PAR_UTEST)
      set( CF3_ENABLED_UTESTS ${CF3_ENABLED_UTESTS} ${_TEST_NAME} CACHE INTERNAL "" )
    elseif(_PAR_ATEST)
      set( CF3_ENABLED_ATESTS ${CF3_ENABLED_ATESTS} ${_TEST_NAME} CACHE INTERNAL "" )
    elseif(_PAR_PTEST)
      set( CF3_ENABLED_PTESTS ${CF3_ENABLED_PTESTS} ${_TEST_NAME} CACHE INTERNAL "" )
    endif()
  else()
    if(_PAR_UTEST)
      set( CF3_DISABLED_UTESTS ${CF3_DISABLED_UTESTS} ${_TEST_NAME} CACHE INTERNAL "" )
    elseif(_PAR_ATEST)
      set( CF3_DISABLED_ATESTS ${CF3_DISABLED_ATESTS} ${_TEST_NAME} CACHE INTERNAL "" )
    elseif(_PAR_PTEST)
      set( CF3_DISABLED_PTESTS ${CF3_DISABLED_PTESTS} ${_TEST_NAME} CACHE INTERNAL "" )
    endif()
  endif()

endfunction( coolfluid_add_test )

##############################################################################
