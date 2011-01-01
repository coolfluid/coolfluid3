################################################################################
# test Component CAction

list( APPEND utest-actions_cflibs coolfluid_actions coolfluid_mesh_sf coolfluid_mesh_gmsh coolfluid_mesh_neutral)
list( APPEND utest-actions_files
  CDummyLoopOperation.hpp
  CDummyLoopOperation.cpp
  utest-actions.cpp )

coolfluid_add_unittest( utest-actions )

# Custom post-build rule (must be defined after adding target):
# Copy the file rotation.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
list( APPEND mesh_files  rotation-tg.neu  rotation-qd.neu  )
foreach( mfile ${mesh_files} )
  add_custom_command(TARGET utest-actions
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${mfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                    )
endforeach()


################################################################################
# proto tests
if(CF_ENABLE_PROTO)

  ################################################################################
  # test Proto operations

  list( APPEND utest-proto-operators_cflibs coolfluid_mesh coolfluid_actions coolfluid_mesh_sf coolfluid_mesh_block coolfluid_testing coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-operators_files  utest-proto-operators.cpp )

  set( utest-proto-operators_profile ON )

  set( utest-proto-operators_condition ${CF_TMP_HAVE_SIMPLECOMM})

  coolfluid_add_unittest( utest-proto-operators )

  ################################################################################

  list( APPEND utest-proto-heat_cflibs coolfluid_mesh coolfluid_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-heat_files
    utest-proto-heat.cpp )
  coolfluid_add_unittest( utest-proto-heat )

  list( APPEND utest-proto-unsteady_cflibs coolfluid_mesh coolfluid_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-unsteady_files
    utest-proto-unsteady.cpp )
  coolfluid_add_unittest( utest-proto-unsteady )

else()

  coolfluid_mark_not_orphan(utest-proto-heat.cpp utest-proto-operators.cpp utest-proto-unsteady.cpp)

endif()
