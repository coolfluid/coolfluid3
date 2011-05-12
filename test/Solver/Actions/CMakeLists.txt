################################################################################
# test Component CAction

list( APPEND utest-actions_cflibs coolfluid_solver_actions coolfluid_mesh_actions coolfluid_mesh_sf coolfluid_mesh_gmsh coolfluid_mesh_neu)
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

  list( APPEND utest-proto-benchmark_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_block coolfluid_testing coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-benchmark_files  utest-proto-benchmark.cpp )

  set( utest-proto-benchmark_profile ON )

  set( utest-proto-benchmark_condition ${CF_TMP_HAVE_SIMPLECOMM})

  coolfluid_add_unit_test( utest-proto-benchmark )

  ################################################################################

  list( APPEND utest-proto-operators_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-operators_files
    utest-proto-operators.cpp )
  coolfluid_add_unit_test( utest-proto-operators )

else()

  coolfluid_mark_not_orphan(utest-proto-benchmark.cpp utest-proto-operators.cpp)

endif()
