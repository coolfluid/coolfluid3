################################################################################
# test Component CAction

list( APPEND utest-solver-actions_cflibs coolfluid_solver_actions coolfluid_mesh_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_gmsh coolfluid_mesh_neu)
list( APPEND utest-solver-actions_files
  CDummyLoopOperation.hpp
  CDummyLoopOperation.cpp
  utest-solver-actions.cpp )

coolfluid_add_unit_test( utest-solver-actions )

list( APPEND mesh_files  rotation-tg-p1.neu  rotation-qd-p1.neu  )
foreach( mfile ${mesh_files} )
  add_custom_command(TARGET utest-solver-actions
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/${mfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                    )
endforeach()

list( APPEND utest-solver-solvesystem_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_math_lss )
list( APPEND utest-solver-solvesystem_files utest-solver-solvesystem.cpp )
coolfluid_add_unit_test( utest-solver-solvesystem )


################################################################################
# proto tests
if(CF_ENABLE_PROTO)

  ################################################################################

  list( APPEND utest-proto-benchmark_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_blockmesh coolfluid_testing coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-benchmark_files  utest-proto-benchmark.cpp )
  if(CMAKE_BUILD_TYPE_CAPS MATCHES "RELEASE")
    list( APPEND utest-proto-benchmark_args 160 160 120 )
  else()
    list( APPEND utest-proto-benchmark_args 16 16 12 )
  endif()
  coolfluid_add_unit_test( utest-proto-benchmark )

  ################################################################################

  list( APPEND utest-proto-operators_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-operators_files utest-proto-operators.cpp )
  coolfluid_add_unit_test( utest-proto-operators )

  ################################################################################

  list( APPEND utest-proto-components_cflibs coolfluid_mesh coolfluid_mesh_lagrangep1 coolfluid_mesh_generation coolfluid_solver_actions coolfluid_solver )
  list( APPEND utest-proto-components_files utest-proto-components.cpp )
  coolfluid_add_unit_test( utest-proto-components )

  ################################################################################

  list( APPEND utest-proto-elements_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_blockmesh coolfluid_solver )
  list( APPEND utest-proto-elements_files utest-proto-elements.cpp )
  coolfluid_add_unit_test( utest-proto-elements )

  ################################################################################

  list( APPEND utest-proto-parallel_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_blockmesh coolfluid_testing coolfluid_mesh_generation coolfluid_solver )
  list( APPEND utest-proto-parallel_files  utest-proto-parallel.cpp )
  if(CMAKE_BUILD_TYPE_CAPS MATCHES "RELEASE")
    list( APPEND utest-proto-parallel_args 160 160 120 )
  else()
    list( APPEND utest-proto-parallel_args 16 16 12 )
  endif()

  set(utest-proto-parallel_mpi_test TRUE)
  set(utest-proto-parallel_mpi_nprocs 4)

  coolfluid_add_unit_test( utest-proto-parallel )

  ################################################################################

else()

  coolfluid_mark_not_orphan(utest-proto-benchmark.cpp utest-proto-operators.cpp utest-proto-components.cpp utest-proto-elements.cpp utest-proto-parallel.cpp)

endif()
