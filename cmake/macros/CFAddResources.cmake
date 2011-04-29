##############################################################################
# macro for adding a acceptance tests
##############################################################################

macro( coolfluid_add_resources RESOURCESNAME )

# copy files to test dir in build tree
foreach( rfile ${${RESOURCESNAME}_files} )

add_custom_command( OUTPUT  ${rfile}
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${rfile} ${RDM_TEST_RUN_DIR}
                   # COMMENT "Copying file ${rfile} to ${RDM_TEST_RUN_DIR}"
)
endforeach()

add_custom_target( ${RESOURCESNAME} ALL DEPENDS ${${RESOURCESNAME}_files} )

endmacro( coolfluid_add_resources )
