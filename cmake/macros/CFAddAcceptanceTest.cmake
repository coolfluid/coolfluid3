##############################################################################
# macro for adding a acceptance tests
##############################################################################

macro( coolfluid_add_acceptance_test ATESTNAME ATESTSCRIPT )

  if( CF_ENABLE_ACCEPTANCE_TESTS AND coolfluid-command_builds )

    # make a diretory to run the test case

    set( TESTDIR ${CMAKE_CURRENT_BINARY_DIR}/${ATESTNAME}.dir )
    
    file( MAKE_DIRECTORY ${TESTDIR} )

    # put the coolfluid-shell script there

    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/${ATESTSCRIPT} ${TESTDIR}/${ATESTSCRIPT} @ONLY)


# copy files to the build directory (not working)
#    foreach( dfile ${${ATESTNAME}_files} )
#      add_custom_command(OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${dfile}
#                         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${dfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
#                         DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${dfile}
#                         COMMENT "copying ${dfile}"
#                        )

#      add_custom_command(TARGET  coolfluid-command
#                         POST_BUILD
#                         COMMAND ${CMAKE_COMMAND} -E make_directory    ${CMAKE_CURRENT_SOURCE_DIR}/${dfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
#                         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${dfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
#                         COMMENT "copying ${dfile}"
#                        )
#    endforeach()

    # Apparently needs cmake 2.8.4 or greater
    add_test( NAME ${ATESTNAME}
              WORKING_DIRECTORY ${TESTDIR}
              COMMAND coolfluid-command -f ${ATESTSCRIPT} )
  endif()

endmacro( coolfluid_add_acceptance_test )
