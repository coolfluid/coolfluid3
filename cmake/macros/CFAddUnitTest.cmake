##############################################################################
# macro for adding a testing application in the project
##############################################################################

macro( coolfluid_prepare_unittest UTESTNAME )

  # option to build it or not (option is advanced and does not appear in the cmake gui)
  option( CF_BUILD_${UTESTNAME} "Build the ${UTESTNAME} testing application" ON )
  mark_as_advanced(CF_BUILD_${UTESTNAME})

  #
  #
  #
  #
  #
  #
  #
  #
  #
  #
  #

  # check if all required modules are present
  set( ${UTESTNAME}_has_all_plugins TRUE )
  foreach( req_plugin ${${UTESTNAME}_requires_plugins} )
    list( FIND CF_PLUGIN_LIST ${req_plugin} pos )
    if( ${pos} EQUAL -1 )
      set( ${UTESTNAME}_has_all_plugins FALSE )
      if( CF_BUILD_${UTESTNAME} )
          coolfluid_log_verbose( "     \# UTEST [${UTESTNAME}] requires plugin [${req_plugin}] which is not present")
      endif()
    endif()
  endforeach()

  set( ${UTESTNAME}_dir ${CMAKE_CURRENT_SOURCE_DIR} )

  # separate the source files and remove them from the orphan list

  coolfluid_separate_sources("${${UTESTNAME}_files}" ${UTESTNAME})

  source_group( Headers FILES ${${UTESTNAME}_headers} )
  source_group( Sources FILES ${${UTESTNAME}_sources} )

  # set condition if not set outside, default is TRUE
  if( DEFINED  ${UTESTNAME}_condition )
    coolfluid_log_verbose("${UTESTNAME} has condition [${${UTESTNAME}_condition}]")
  else()
    set( ${UTESTNAME}_condition TRUE )
  endif()

  if( CF_ENABLE_UNITTESTS AND CF_BUILD_${UTESTNAME} AND ${UTESTNAME}_has_all_plugins AND ${UTESTNAME}_condition )
    set( ${UTESTNAME}_builds YES CACHE INTERNAL "" )
  else()
    set( ${UTESTNAME}_builds NO  CACHE INTERNAL "" )
  endif()

  # compile if selected and all required modules are present
  if( ${UTESTNAME}_builds )

    coolfluid_log( " +++ UTEST [${UTESTNAME}]" )

    if( DEFINED ${UTESTNAME}_includedirs )
      include_directories(${${UTESTNAME}_includedirs})
    endif()

    if( DEFINED ${UTESTNAME}_moc_files )
      add_executable( ${UTESTNAME} ${${UTESTNAME}_sources} ${${UTESTNAME}_headers}  ${${UTESTNAME}_moc_files})
    else()
      add_executable( ${UTESTNAME} ${${UTESTNAME}_sources} ${${UTESTNAME}_headers})
    endif()

    if(CF_INSTALL_TESTS)  # add installation paths
      install( TARGETS ${UTESTNAME}
        RUNTIME DESTINATION ${CF_INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${CF_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${CF_INSTALL_LIB_DIR}
        )
    endif(CF_INSTALL_TESTS)

    # if mpi was found add it to the libraries
    if(CF_HAVE_MPI AND NOT CF_HAVE_MPI_COMPILER)
#           message( STATUS "${UTESTNAME} links to ${MPI_LIBRARIES}" )
          TARGET_LINK_LIBRARIES ( ${UTESTNAME} ${MPI_LIBRARIES} )
    endif()

    # add external dependency libraries if defined
    if( DEFINED ${UTESTNAME}_libs )
      target_link_libraries( ${UTESTNAME} ${${UTESTNAME}_libs} )
    endif(DEFINED ${UTESTNAME}_libs)

    # profiling gloabally selected
    if( CF_ENABLE_PROFILING AND CF_PROFILER_IS_GOOGLE AND CF_BUILD_coolfluid_google_perftools )
      list( APPEND ${UTESTNAME}_cflibs coolfluid_google_perftools )
    endif()

    # profiling selected for specific target
    if( ${UTESTNAME}_profile AND CF_BUILD_coolfluid_google_perftools )
      list( APPEND ${UTESTNAME}_cflibs coolfluid_google_perftools )
    endif()

    # internal dependencies
    if( DEFINED ${UTESTNAME}_cflibs )
        target_link_libraries( ${UTESTNAME} ${${UTESTNAME}_cflibs} )
    endif()

    # add boost unit test lib
    target_link_libraries( ${UTESTNAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )

  endif()

  get_target_property( ${UTESTNAME}_P_SOURCES   ${UTESTNAME} SOURCES )
  get_target_property( ${UTESTNAME}_LINK_FLAGS       ${UTESTNAME} LINK_FLAGS )
  get_target_property( ${UTESTNAME}_TYPE             ${UTESTNAME} TYPE )

  # log some info about the unit test
  coolfluid_log_file("${UTESTNAME} user option     : [${CF_BUILD_${UTESTNAME}}]")
  coolfluid_log_file("${UTESTNAME}_builds          : [${${UTESTNAME}_builds}]")
  coolfluid_log_file("${UTESTNAME}_dir             : [${${UTESTNAME}_dir}]")
  coolfluid_log_file("${UTESTNAME}_includedirs     : [${${UTESTNAME}_includedirs}]")
  coolfluid_log_file("${UTESTNAME}_libs            : [${${UTESTNAME}_libs}]")
  coolfluid_log_file("${UTESTNAME}_cflibs          : [${${UTESTNAME}_cflibs}]")
  coolfluid_log_file("${UTESTNAME}_has_all_plugins : [${${UTESTNAME}_has_all_plugins}]")
  coolfluid_log_file("${UTESTNAME}_requires_plugins: [${${UTESTNAME}_requires_plugins}]")
  coolfluid_log_file("${UTESTNAME}_P_SOURCES       : [${${UTESTNAME}_P_SOURCES}]")
  coolfluid_log_file("${UTESTNAME}_LINK_FLAGS      : [${${UTESTNAME}_LINK_FLAGS}]")
  coolfluid_log_file("${UTESTNAME}_TYPE            : [${${UTESTNAME}_TYPE}]")


endmacro( coolfluid_prepare_unittest )

##############################################################################
##############################################################################

macro( coolfluid_add_unittest UTESTNAME )

coolfluid_prepare_unittest(${UTESTNAME})
if(${UTESTNAME}_builds)
  # add to the test database
  add_test( ${UTESTNAME} ${UTESTNAME} ${${UTESTNAME}_args})
endif()

endmacro( coolfluid_add_unittest )

##############################################################################
##############################################################################

macro( coolfluid_add_mpi_unittest UTESTNAME NUMBER_PROC)

if(CF_MPI_TESTS_RUN)
  coolfluid_prepare_unittest(${UTESTNAME})
  if(${UTESTNAME}_builds)
    # add to the test database
    #${CF_MPI_TESTS_NB_PROCS}
    add_test( ${UTESTNAME} ${CF_MPIRUN_PROGRAM} "-np" ${NUMBER_PROC} ${UTESTNAME} ${${UTESTNAME}_args})
    if(CF_MPI_TESTS_RUN_SCALABILITY AND ${UTESTNAME}_scaling)
      add_test("${UTESTNAME}-scaling" ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/test-mpi-scalability.py ${CF_MPIRUN_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${UTESTNAME} ${CF_MPI_TESTS_MAX_NB_PROCS} ${${UTESTNAME}_args})
    endif()
  endif()
else(CF_MPI_TESTS_RUN)
  coolfluid_mark_not_orphan(${${UTESTNAME}_files})
endif(CF_MPI_TESTS_RUN)

endmacro( coolfluid_add_mpi_unittest )


##############################################################################
##############################################################################
