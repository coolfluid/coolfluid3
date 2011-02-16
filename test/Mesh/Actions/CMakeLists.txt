################################################################################

list( APPEND utest-mesh-actions-facebuilder_cflibs coolfluid_mesh_actions coolfluid_mesh_neutral coolfluid_mesh_gmsh)
list( APPEND utest-mesh-actions-facebuilder_files  utest-mesh-actions-facebuilder.cpp )

coolfluid_add_unittest( utest-mesh-actions-facebuilder )

add_custom_command(TARGET utest-mesh-actions-facebuilder
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-bubble-enrich_cflibs coolfluid_mesh_actions coolfluid_mesh_sf coolfluid_mesh_gmsh)
list( APPEND utest-bubble-enrich_files  utest-bubble-enrich.cpp )

coolfluid_add_unittest( utest-bubble-enrich )

add_custom_command(TARGET utest-bubble-enrich
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rectangle-tg-p2.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/advection-tg-p2.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/rectangle-tg-p1.msh ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )
