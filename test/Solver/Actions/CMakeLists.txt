################################################################################
# test Component CAction

list( APPEND utest-actions_cflibs coolfluid_solver_actions coolfluid_mesh_actions coolfluid_mesh_sf coolfluid_mesh_gmsh coolfluid_mesh_neutral)
list( APPEND utest-actions_files
  CDummyLoopOperation.hpp
  CDummyLoopOperation.cpp
  utest-actions.cpp )

coolfluid_add_unit_test( utest-actions )

list( APPEND mesh_files  rotation-tg-p1.neu  rotation-qd-p1.neu  )
foreach( mfile ${mesh_files} )
  add_custom_command(TARGET utest-actions
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/${mfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                    )
endforeach()


################################################################################
# proto tests
if(CF_ENABLE_PROTO)

  ################################################################################
  # test Proto operations

  list( APPEND utest-proto-operators_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_block coolfluid_testing coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-operators_files  utest-proto-operators.cpp )

  set( utest-proto-operators_profile ON )

  set( utest-proto-operators_condition ${CF_TMP_HAVE_SIMPLECOMM})

  coolfluid_add_unit_test( utest-proto-operators )

  ################################################################################

  set( utest-proto-heat_condition ${CF_HAVE_SUPERLU} )

  list( APPEND utest-proto-heat_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-heat_files
    utest-proto-heat.cpp )
  coolfluid_add_unit_test( utest-proto-heat )

  set( utest-proto-unsteady_condition ${CF_HAVE_SUPERLU} )

  list( APPEND utest-proto-unsteady_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-unsteady_files
    utest-proto-unsteady.cpp )
  coolfluid_add_unit_test( utest-proto-unsteady )

  list( APPEND utest-proto-systems_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-systems_files
    utest-proto-systems.cpp )
  coolfluid_add_unit_test( utest-proto-systems )

else()

  coolfluid_mark_not_orphan(utest-proto-heat.cpp utest-proto-operators.cpp utest-proto-systems.cpp utest-proto-unsteady.cpp)

endif()
