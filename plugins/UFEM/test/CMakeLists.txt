if (${coolfluid_ufem_builds} AND ${CF_HAVE_TRILINOS} )
    set ( ufem_tests_trilinos_condition TRUE)
else()
    set ( ufem_tests_trilinos_condition FALSE)
endif()

list( APPEND utest-proto-heat_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem)
list( APPEND utest-proto-heat_files
  utest-proto-heat.cpp )
list( APPEND utest-proto-heat_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( utest-proto-heat_mpi_test TRUE )
set( utest-proto-heat_mpi_nprocs 1 )
set( utest-proto-heat_condition ${coolfluid_ufem_builds} )
coolfluid_add_unit_test( utest-proto-heat )

list( APPEND utest-proto-heat-parallel_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem coolfluid_mesh_blockmesh)
list( APPEND utest-proto-heat-parallel_files
  utest-proto-heat-parallel.cpp )
list( APPEND utest-proto-heat-parallel_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( utest-proto-heat-parallel_mpi_test TRUE )
set( utest-proto-heat-parallel_mpi_nprocs 4 )
set( utest-proto-heat-parallel_condition ${coolfluid_ufem_builds} )
coolfluid_add_unit_test( utest-proto-heat-parallel )

list( APPEND utest-proto-unsteady_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem)
list( APPEND utest-proto-unsteady_files
  utest-proto-unsteady.cpp )
list( APPEND utest-proto-unsteady_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( utest-proto-unsteady_mpi_test TRUE )
set( utest-proto-unsteady_mpi_nprocs 1 )
set( utest-proto-unsteady_condition ${coolfluid_ufem_builds} )
coolfluid_add_unit_test( utest-proto-unsteady )

list( APPEND utest-proto-systems_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem)
list( APPEND utest-proto-systems_files
  utest-proto-systems.cpp )
list( APPEND utest-proto-systems_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( utest-proto-systems_mpi_test TRUE )
set( utest-proto-systems_mpi_nprocs 1 )
set( utest-proto-systems_condition ${coolfluid_ufem_builds} )
coolfluid_add_unit_test( utest-proto-systems )

list( APPEND utest-proto-navier-stokes_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem)
list( APPEND utest-proto-navier-stokes_files
  NavierStokes.cpp
  NavierStokes.hpp
  utest-proto-navier-stokes.cpp )
list( APPEND utest-proto-navier-stokes_args ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( utest-proto-navier-stokes_mpi_test TRUE )
set( utest-proto-navier-stokes_mpi_nprocs 1 )
set( utest-proto-navier-stokes_condition ${coolfluid_ufem_builds} )
coolfluid_add_unit_test( utest-proto-navier-stokes )

list( APPEND utest-ufem-buildsparsity_cflibs coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem coolfluid_mesh_blockmesh)
list( APPEND utest-ufem-buildsparsity_files
  utest-ufem-buildsparsity.cpp )
set( utest-ufem-buildsparsity_mpi_test TRUE )
set( utest-ufem-buildsparsity_mpi_nprocs 1 )
set( utest-ufem-buildsparsity_condition ${coolfluid_ufem_builds} )
coolfluid_add_unit_test( utest-ufem-buildsparsity )

# Disable debugging on the compiled expressions, since this takes huge amounts of memory
set_source_files_properties(NavierStokes.cpp PROPERTIES COMPILE_FLAGS "-g0")

set( UFEM_LSS_CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( UFEM_HEAT_INPUT_MESH ${CMAKE_CURRENT_SOURCE_DIR}/meshes/ring2d-quads.neu )

coolfluid_add_acceptance_test( NAME    atest-ufem-heat-steady
                               SCRIPT  atest-ufem-heat-steady.cfscript )

coolfluid_add_python_test(ATEST NAME atest-ufem-python
                                SCRIPT atest-ufem-python.py
                                ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/ring2d-quads.neu ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml)

coolfluid_add_python_test(ATEST NAME atest-ufem-heat3d-channel
                                SCRIPT atest-ufem-heat3d-channel.py
                                ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                                MPI NBPROCS 4)

coolfluid_add_python_test(ATEST NAME atest-ufem-heat3d-sphere
                                SCRIPT atest-ufem-heat3d-sphere.py
                                ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/ring3d-tetras.neu ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                                MPI NBPROCS 4)

coolfluid_add_python_test(ATEST NAME atest-ufem-navier-stokes-cylinder2d
                                SCRIPT atest-ufem-navier-stokes-cylinder2d.py
                                ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/kvs15.neu ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                                MPI NBPROCS 4)

coolfluid_mark_not_orphan(tris.cpp)