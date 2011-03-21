list( APPEND utest-heat-steady_cflibs coolfluid_mesh)
list( APPEND utest-heat-steady_files
  utest-heat-steady.cpp )

list( APPEND utest-heat-steady_args ${CMAKE_CURRENT_SOURCE_DIR}/meshes )

set( utest-heat-steady_condition ${coolfluid_ufem_builds} )

coolfluid_add_unit_test( utest-heat-steady )

list( APPEND utest-heat-steady-tetra_cflibs coolfluid_mesh)
list( APPEND utest-heat-steady-tetra_files
  utest-heat-steady-tetra.cpp )

list( APPEND utest-heat-steady-tetra_args ${CMAKE_CURRENT_SOURCE_DIR}/meshes )

set( utest-heat-steady-tetra_condition ${coolfluid_ufem_builds} )

coolfluid_add_unit_test( utest-heat-steady-tetra )

list( APPEND utest-heat-unsteady_cflibs coolfluid_mesh )
list( APPEND utest-heat-unsteady_files
  utest-heat-unsteady.cpp )

list( APPEND utest-heat-unsteady_args ${CMAKE_CURRENT_SOURCE_DIR}/meshes )

set( utest-heat-unsteady_condition ${coolfluid_ufem_builds} )

coolfluid_add_unit_test( utest-heat-unsteady )

list( APPEND utest-proto-heat_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-heat_files
  utest-proto-heat.cpp )
list( APPEND utest-proto-heat_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
coolfluid_add_unit_test( utest-proto-heat )

list( APPEND utest-proto-unsteady_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-unsteady_files
  utest-proto-unsteady.cpp )
list( APPEND utest-proto-unsteady_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
coolfluid_add_unit_test( utest-proto-unsteady )

list( APPEND utest-proto-systems_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-systems_files
  utest-proto-systems.cpp )
list( APPEND utest-systems_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
coolfluid_add_unit_test( utest-proto-systems )

list( APPEND utest-proto-stokes-artifdiss_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-stokes-artifdiss_files
  utest-proto-stokes-artifdiss.cpp )
list( APPEND utest-stokes-artifdiss_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
coolfluid_add_unit_test( utest-proto-stokes-artifdiss )

list( APPEND utest-proto-stokes-pspg_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-stokes-pspg_files
  utest-proto-stokes-pspg.cpp )
list( APPEND utest-stokes-pspg_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
coolfluid_add_unit_test( utest-proto-stokes-pspg )

list( APPEND utest-proto-navier-stokes-pspg_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_solver )
list( APPEND utest-proto-navier-stokes-pspg_files
  utest-proto-navier-stokes-pspg.cpp )
list( APPEND utest-proto-navier-stokes-pspg_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
list( APPEND utest-proto-navier-stokes-pspg_args ${CMAKE_CURRENT_SOURCE_DIR}/kvs.neu )
coolfluid_add_unit_test( utest-proto-navier-stokes-pspg )
