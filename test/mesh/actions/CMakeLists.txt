################################################################################

list( APPEND utest-mesh-actions-facebuilder_cflibs coolfluid_mesh_actions coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1)
list( APPEND utest-mesh-actions-facebuilder_files  utest-mesh-actions-facebuilder.cpp )

coolfluid_add_unit_test( utest-mesh-actions-facebuilder )

add_custom_command(TARGET utest-mesh-actions-facebuilder
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF3_RESOURCES_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND utest-mesh-actions-global-connectivity_cflibs coolfluid_mesh_actions coolfluid_mesh_neu coolfluid_mesh_gmsh coolfluid_mesh_lagrangep1)
list( APPEND utest-mesh-actions-global-connectivity_files  utest-mesh-actions-global-connectivity.cpp )

set( utest-mesh-actions-global-connectivity_mpi_test TRUE )
set( utest-mesh-actions-global-connectivity_mpi_nprocs 2)
coolfluid_add_unit_test( utest-mesh-actions-global-connectivity )

add_custom_command(TARGET utest-mesh-actions-global-connectivity
                 POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF3_RESOURCES_DIR}/quadtriag.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                )

################################################################################

coolfluid_add_test( UTEST utest-mesh-actions-interpolate
                    CPP   utest-mesh-actions-interpolate.cpp
                    LIBS  coolfluid_mesh_actions coolfluid_mesh_lagrangep1
                    MPI   2 )
