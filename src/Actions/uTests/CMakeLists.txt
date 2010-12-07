################################################################################
# test Component CAction

list( APPEND utest-actions_cflibs coolfluid_actions coolfluid_mesh_sf coolfluid_gmsh coolfluid_neutral)
list( APPEND utest-actions_files
  utest-actions.cpp )

coolfluid_add_unittest( utest-actions )

# Custom post-build rule (must be defined after adding target):
# Copy the file rotation.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET utest-actions
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation-tg.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

add_custom_command(TARGET utest-actions
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation-qd.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

add_custom_command(TARGET utest-actions
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation-qd.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )
if(CF_ENABLE_PROTO)
  ################################################################################
# test Proto operations

list( APPEND utest-proto-operators_cflibs coolfluid_mesh coolfluid_actions coolfluid_mesh_sf coolfluid_blockmesh coolfluid_testing coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-operators_files
  utest-proto-operators.cpp )
set( utest-proto-operators_profile ON )
coolfluid_add_unittest( utest-proto-operators )

################################################################################

list( APPEND utest-proto-heat_cflibs coolfluid_mesh coolfluid_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-heat_files
  utest-proto-heat.cpp )
coolfluid_add_unittest( utest-proto-heat )

else()

coolfluid_mark_not_orphan(utest-proto-heat.cpp utest-proto-operators.cpp)

endif()
