set( CF_RESOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Dir for resources" )

list( APPEND resources_files
   advection_p2.msh
   advection-p2-quad.msh
   hextet.neu
   quadtriag.neu
   rotation-qd.neu
   rotation-tg.neu
   rectangle-tg-p1.msh
   rectangle-tg-p2.msh
   rectangle-qd-p1.msh
   rectangle-qd-p2.msh
   rectangle-mix-p1.msh
   rectangle-mix-p2.msh
   )

add_custom_target( resources SOURCES ${resources_files} )

# copy files to build tree
foreach( rfile ${resources_files} )
  add_custom_command(TARGET resources
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/${mfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                    )
endforeach()
