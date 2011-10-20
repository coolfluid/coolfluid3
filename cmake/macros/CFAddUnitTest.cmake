##############################################################################
# macro for adding a unit test
##############################################################################

macro( coolfluid_prepare_test UTESTNAME )

  # option to build it or not (option is advanced and does not appear in the cmake gui)
  option( CF3_BUILD_${UTESTNAME} "Build the ${UTESTNAME} testing application" ON )
  mark_as_advanced(CF3_BUILD_${UTESTNAME})

  if( DEFINED ${UTESTNAME}_performance_test )
    set( ${UTESTNAME}_performance_test ${${UTESTNAME}_performance_test} CACHE INTERNAL "" )
  else()
    set( ${UTESTNAME}_performance_test FALSE CACHE INTERNAL "" )
  endif()
  #
  #
  #
  #
  #

  # check if all required modules are present
  set( ${UTESTNAME}_has_all_plugins TRUE )
  foreach( req_plugin ${${UTESTNAME}_requires_plugins} )
    list( FIND CF3_PLUGIN_LIST ${req_plugin} pos )
    if( ${pos} EQUAL -1 )
      set( ${UTESTNAME}_has_all_plugins FALSE )
      if( CF3_BUILD_${UTESTNAME} )
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

  if( CF3_ENABLE_UNIT_TESTS AND CF3_BUILD_${UTESTNAME} AND ${UTESTNAME}_has_all_plugins AND ${UTESTNAME}_condition )
    set( ${UTESTNAME}_builds YES CACHE INTERNAL "" )
  else()
    set( ${UTESTNAME}_builds NO  CACHE INTERNAL "" )
  endif()

  # compile if selected and all required modules are present
  if( ${UTESTNAME}_builds )

    coolfluid_log_file( " +++ UTEST [${UTESTNAME}]" )

    if( DEFINED ${UTESTNAME}_includedirs )
      include_directories(${${UTESTNAME}_includedirs})
    endif()

    if( DEFINED ${UTESTNAME}_moc_files )
      add_executable( ${UTESTNAME} ${${UTESTNAME}_sources} ${${UTESTNAME}_headers}  ${${UTESTNAME}_moc_files})
    else()
      add_executable( ${UTESTNAME} ${${UTESTNAME}_sources} ${${UTESTNAME}_headers})
    endif()


    if( DEFINED  ${UTESTNAME}_resources )
      foreach( resource ${${UTESTNAME}_resources} )
        add_custom_command(TARGET ${UTESTNAME}
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E copy_if_different ${resource} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                          )
      endforeach()
    endif()

    if(CF3_INSTALL_TESTS)  # add installation paths
      install( TARGETS ${UTESTNAME}
        RUNTIME DESTINATION ${CF3_INSTALL_BIN_DIR}
        LIBRARY DESTINATION ${CF3_INSTALL_LIB_DIR}
        ARCHIVE DESTINATION ${CF3_INSTALL_LIB_DIR}
        )
    endif(CF3_INSTALL_TESTS)

    # if mpi was found add it to the libraries
    if(CF3_HAVE_MPI AND NOT CF3_HAVE_MPI_COMPILER)
#           message( STATUS "${UTESTNAME} links to ${MPI_LIBRARIES}" )
          TARGET_LINK_LIBRARIES ( ${UTESTNAME} ${MPI_LIBRARIES} )
    endif()

    # add external dependency libraries if defined
    if( DEFINED ${UTESTNAME}_libs )
      #list(REMOVE_DUPLICATES ${UTESTNAME}_libs)
      target_link_libraries( ${UTESTNAME} ${${UTESTNAME}_libs} )
    endif(DEFINED ${UTESTNAME}_libs)

    # internal dependencies
    if( DEFINED ${UTESTNAME}_cflibs )
        list(REMOVE_DUPLICATES ${UTESTNAME}_cflibs)
        target_link_libraries( ${UTESTNAME} ${${UTESTNAME}_cflibs} )
    endif()

    # add boost unit test lib
    target_link_libraries( ${UTESTNAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )

  endif()

  get_target_property( ${UTESTNAME}_P_SOURCES   ${UTESTNAME} SOURCES )
  get_target_property( ${UTESTNAME}_LINK_FLAGS       ${UTESTNAME} LINK_FLAGS )
  get_target_property( ${UTESTNAME}_TYPE             ${UTESTNAME} TYPE )

  # log some info about the unit test
  coolfluid_log_file("${UTESTNAME} user option     : [${CF3_BUILD_${UTESTNAME}}]")
  coolfluid_log_file("${UTESTNAME}_builds          : [${${UTESTNAME}_builds}]")
  coolfluid_log_file("${UTESTNAME}_dir             : [${${UTESTNAME}_dir}]")
  coolfluid_log_file("${UTESTNAME}_includedirs     : [${${UTESTNAME}_includedirs}]")
  coolfluid_log_file("${UTESTNAME}_libs            : [${${UTESTNAME}_libs}]")
  coolfluid_log_file("${UTESTNAME}_cflibs          : [${${UTESTNAME}_cflibs}]")
  coolfluid_log_file("${UTESTNAME}_has_all_plugins : [${${UTESTNAME}_has_all_plugins}]")
  coolfluid_log_file("${UTESTNAME}_requires_plugins: [${${UTESTNAME}_requires_plugins}]")
  coolfluid_log_file("${UTESTNAME}_resources:        [${${UTESTNAME}_resources}]")
  coolfluid_log_file("${UTESTNAME}_P_SOURCES       : [${${UTESTNAME}_P_SOURCES}]")
  coolfluid_log_file("${UTESTNAME}_LINK_FLAGS      : [${${UTESTNAME}_LINK_FLAGS}]")
  coolfluid_log_file("${UTESTNAME}_TYPE            : [${${UTESTNAME}_TYPE}]")


endmacro( coolfluid_prepare_test )

##############################################################################
##############################################################################

macro( coolfluid_add_unit_test UTESTNAME )

coolfluid_prepare_test(${UTESTNAME})

if( ${${UTESTNAME}_builds} ) # dont add if it does not build

  if( ${UTESTNAME}_performance_test AND NOT CF3_ENABLE_PERFORMANCE_TESTS )
    set( ${UTESTNAME}_performance_skip TRUE )
  else()
    set( ${UTESTNAME}_performance_skip FALSE )
  endif()

  if( CF3_ALL_UTESTS_PARALLEL )
    set( ${UTESTNAME}_mpi_test TRUE )
  endif()

  if( ${UTESTNAME}_mpi_test AND NOT CF3_MPI_TESTS_RUN )
    set( ${UTESTNAME}_mpi_skip TRUE )
  else()
    set( ${UTESTNAME}_mpi_skip FALSE )
  endif()

  # add to the test database
  if( NOT ${UTESTNAME}_performance_skip AND NOT ${UTESTNAME}_mpi_skip )

    if( NOT ${UTESTNAME}_mpi_test )
    # standard test

      add_test( ${UTESTNAME} ${UTESTNAME} ${${UTESTNAME}_args})

    else()
    # mpi test

      if( NOT DEFINED ${UTESTNAME}_mpi_nprocs )
         set(${UTESTNAME}_mpi_nprocs "1")
      endif()

      add_test( ${UTESTNAME} ${CF3_MPIRUN_PROGRAM} "-np" ${${UTESTNAME}_mpi_nprocs} ${UTESTNAME} ${${UTESTNAME}_args} )
      if(CF3_MPI_TESTS_RUN_SCALABILITY AND ${UTESTNAME}_scaling)
        add_test("${UTESTNAME}-scaling" ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/test-mpi-scalability.py ${CF3_MPIRUN_PROGRAM} ${CMAKE_CURRENT_BINARY_DIR}/${UTESTNAME} ${CF3_MPI_TESTS_MAX_NB_PROCS} ${${UTESTNAME}_args})
      endif()

    endif()

  endif() # mpi and performance skip

endif() # build guard

endmacro( coolfluid_add_unit_test )

