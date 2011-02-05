################################################################################

list( APPEND utest-mesh-actions-facebuilder_cflibs coolfluid_mesh_actions coolfluid_mesh_neutral coolfluid_mesh_gmsh)
list( APPEND utest-mesh-actions-facebuilder_files  utest-mesh-actions-facebuilder.cpp )

coolfluid_add_unittest( utest-mesh-actions-facebuilder )

add_custom_command(TARGET utest-mesh-actions-facebuilder
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )
