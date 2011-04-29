##############################################################################
# macro for adding resources
##############################################################################

macro( coolfluid_add_resources RESOURCESNAME )

set( ${RESOURCESNAME}_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} CACHE INTERNAL "build dir" )

# copy files to test dir in build tree
foreach( rfile ${${RESOURCESNAME}_files} )

add_custom_command( OUTPUT  ${rfile}
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${rfile} ${RDM_TEST_RUN_DIR}
                   # COMMENT "Copying file ${rfile} to ${RDM_TEST_RUN_DIR}"
)
endforeach()

add_custom_target( ${RESOURCESNAME} ALL DEPENDS ${${RESOURCESNAME}_files} )

endmacro( coolfluid_add_resources )
