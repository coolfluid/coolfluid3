##############################################################################
# macro for adding a acceptance tests
##############################################################################

macro( coolfluid_add_acceptance_test ATESTNAME ATESTSCRIPT )

  if( CF_ENABLE_ACCEPTANCE_TESTS AND coolfluid-command_builds )

    # make a diretory to run the test case

#    set( TESTDIR ${CMAKE_CURRENT_BINARY_DIR}/${ATESTNAME}.dir )
    set( TESTDIR ${CMAKE_CURRENT_BINARY_DIR} )

#    file( MAKE_DIRECTORY ${TESTDIR} )

    # put the coolfluid-shell script there

    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${ATESTSCRIPT} ${TESTDIR}/${ATESTSCRIPT} @ONLY)

    # Apparently needs cmake 2.8.4 or greater
    add_test( NAME ${ATESTNAME}
 #             WORKING_DIRECTORY ${TESTDIR}
              COMMAND coolfluid-command -f ${ATESTSCRIPT} )
  endif()

endmacro( coolfluid_add_acceptance_test )
