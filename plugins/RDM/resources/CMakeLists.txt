set( RDM_RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Dir for resources" )

list( APPEND resources_files
 rotation-qd-p1.msh 
 rotation-qd-p2.msh 
 rotation-qd-p3.msh 
 rotation-qd-p4.msh 
 rotation-tg-p1.msh 
 rotation-tg-p2.msh 
 rotation-tg-p3.msh 
 rotation-tg-p4.msh
)

# add_custom_target( resources SOURCES ${resources_files} )
#
# copy files to build tree
# foreach( rfile ${resources_files} )
#  add_custom_command(TARGET resources
#                     POST_BUILD
#                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/${mfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
#                    )
# endforeach()

