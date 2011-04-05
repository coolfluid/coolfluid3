list( APPEND utest-heat-steady_cflibs coolfluid_mesh)
list( APPEND utest-heat-steady_files
  utest-heat-steady.cpp )

list( APPEND utest-heat-steady_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml ${CMAKE_CURRENT_SOURCE_DIR}/meshes )
set( utest-heat-steady_mpi_test TRUE )
set( utest-heat-steady_mpi_nprocs 1 )
set( utest-heat-steady_condition ${coolfluid_ufem_builds} )

coolfluid_add_unit_test( utest-heat-steady )

list( APPEND utest-heat-steady-tetra_cflibs coolfluid_mesh)
list( APPEND utest-heat-steady-tetra_files
  utest-heat-steady-tetra.cpp )
set( utest-heat-steady-tetra_mpi_test TRUE )
set( utest-heat-steady-tetra_mpi_nprocs 1 )
list( APPEND utest-heat-steady-tetra_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml ${CMAKE_CURRENT_SOURCE_DIR}/meshes )

set( utest-heat-steady-tetra_condition ${coolfluid_ufem_builds} AND ${CF_HAVE_TRILINOS} )

coolfluid_add_unit_test( utest-heat-steady-tetra )

list( APPEND utest-heat-unsteady_cflibs coolfluid_mesh )
list( APPEND utest-heat-unsteady_files
  utest-heat-unsteady.cpp )

list( APPEND utest-heat-unsteady_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml ${CMAKE_CURRENT_SOURCE_DIR}/meshes )
set( utest-heat-unsteady_mpi_test TRUE )
set( utest-heat-unsteady_mpi_nprocs 1 )

set( utest-heat-unsteady_condition ${coolfluid_ufem_builds} )

coolfluid_add_unit_test( utest-heat-unsteady )

list( APPEND utest-proto-heat_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-heat_files
  utest-proto-heat.cpp )
list( APPEND utest-proto-heat_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( utest-proto-heat_mpi_test TRUE )
set( utest-proto-heat_mpi_nprocs 1 )
coolfluid_add_unit_test( utest-proto-heat )

list( APPEND utest-proto-unsteady_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-unsteady_files
  utest-proto-unsteady.cpp )
list( APPEND utest-proto-unsteady_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( utest-proto-unsteady_mpi_test TRUE )
set( utest-proto-unsteady_mpi_nprocs 1 )
coolfluid_add_unit_test( utest-proto-unsteady )

list( APPEND utest-proto-systems_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-systems_files
  utest-proto-systems.cpp )
list( APPEND utest-proto-systems_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( utest-proto-systems_mpi_test TRUE )
set( utest-proto-systems_mpi_nprocs 1 )
coolfluid_add_unit_test( utest-proto-systems )

list( APPEND utest-proto-stokes-artifdiss_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-stokes-artifdiss_files
  utest-proto-stokes-artifdiss.cpp )
list( APPEND utest-proto-stokes-artifdiss_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( utest-proto-stokes-artifdiss_mpi_test TRUE )
set( utest-proto-stokes-artifdiss_mpi_nprocs 1 )
coolfluid_add_unit_test( utest-proto-stokes-artifdiss )

list( APPEND utest-proto-stokes-pspg_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-stokes-pspg_files
  utest-proto-stokes-pspg.cpp )
list( APPEND utest-proto-stokes-pspg_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( utest-proto-stokes-pspg_mpi_test TRUE )
set( utest-proto-stokes-pspg_mpi_nprocs 1 )
coolfluid_add_unit_test( utest-proto-stokes-pspg )

list( APPEND utest-proto-navier-stokes-pspg_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_solver )
list( APPEND utest-proto-navier-stokes-pspg_files
  utest-proto-navier-stokes-pspg.cpp )
list( APPEND utest-proto-navier-stokes-pspg_args 0.1 ) # end time
list( APPEND utest-proto-navier-stokes-pspg_args 0.1 ) # time step
list( APPEND utest-proto-navier-stokes-pspg_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
list( APPEND utest-proto-navier-stokes-pspg_args ${CMAKE_CURRENT_SOURCE_DIR}/kvs.neu )
set( utest-proto-navier-stokes-pspg_mpi_test TRUE )
set( utest-proto-navier-stokes-pspg_mpi_nprocs 1 )
set( utest-proto-navier-stokes-pspg_condition ${CF_HAVE_TRILINOS} )
coolfluid_add_unit_test( utest-proto-navier-stokes-pspg )

list( APPEND utest-proto-navier-stokes-supg_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_mesh_generation coolfluid_solver )
list( APPEND utest-proto-navier-stokes-supg_files
  utest-proto-navier-stokes-supg.cpp )
list( APPEND utest-proto-navier-stokes-supg_args 0.01 ) # end time
list( APPEND utest-proto-navier-stokes-supg_args 0.01 ) # time step
list( APPEND utest-proto-navier-stokes-supg_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
list( APPEND utest-proto-navier-stokes-supg_args ${CMAKE_CURRENT_SOURCE_DIR}/kvs.neu )
set( utest-proto-navier-stokes-supg_mpi_test TRUE )
set( utest-proto-navier-stokes-supg_mpi_nprocs 1 )
set( utest-proto-navier-stokes-supg_condition ${CF_HAVE_TRILINOS} )
coolfluid_add_unit_test( utest-proto-navier-stokes-supg )

list( APPEND utest-proto-navier-stokes-bulk_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_solver )
list( APPEND utest-proto-navier-stokes-bulk_files
  utest-proto-navier-stokes-bulk.cpp )
list( APPEND utest-proto-navier-stokes-bulk_args 0.01 ) # end time
list( APPEND utest-proto-navier-stokes-bulk_args 0.01 ) # time step
list( APPEND utest-proto-navier-stokes-bulk_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
list( APPEND utest-proto-navier-stokes-bulk_args ${CMAKE_CURRENT_SOURCE_DIR}/kvs.neu )
set( utest-proto-navier-stokes-bulk_mpi_test TRUE )
set( utest-proto-navier-stokes-bulk_mpi_nprocs 1 )
set( utest-proto-navier-stokes-bulk_condition ${CF_HAVE_TRILINOS} )
coolfluid_add_unit_test( utest-proto-navier-stokes-bulk )

list( APPEND utest-proto-navier-stokes-linearized_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_sf coolfluid_solver )
list( APPEND utest-proto-navier-stokes-linearized_files
  utest-proto-navier-stokes-linearized.cpp )
list( APPEND utest-proto-navier-stokes-linearized_args 0.01 ) # end time
list( APPEND utest-proto-navier-stokes-linearized_args 0.01 ) # time step
list( APPEND utest-proto-navier-stokes-linearized_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
list( APPEND utest-proto-navier-stokes-linearized_args ${CMAKE_CURRENT_SOURCE_DIR}/kvs.neu )
set( utest-proto-navier-stokes-linearized_mpi_test TRUE )
set( utest-proto-navier-stokes-linearized_mpi_nprocs 1 )
set( utest-proto-navier-stokes-linearized_condition ${CF_HAVE_TRILINOS} )
coolfluid_add_unit_test( utest-proto-navier-stokes-linearized )