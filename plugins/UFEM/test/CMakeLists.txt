# UFEM tests
coolfluid_add_test( UTEST utest-proto-heat
                    CPP utest-proto-heat.cpp
                    LIBS coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem
                    MPI 1)

coolfluid_add_test( UTEST utest-proto-heat-parallel
                    CPP utest-proto-heat-parallel.cpp
                    LIBS coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem coolfluid_mesh_blockmesh
                    MPI 4)

coolfluid_add_test( UTEST utest-proto-unsteady
                    CPP utest-proto-unsteady.cpp
                    LIBS coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem
                    MPI 1)

coolfluid_add_test( UTEST utest-proto-systems
                    CPP utest-proto-systems.cpp
                    LIBS coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem
                    MPI 1)

coolfluid_add_test( UTEST utest-proto-navier-stokes
                    CPP utest-proto-navier-stokes.cpp NavierStokes.hpp NavierStokes.cpp
                    LIBS coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem
                    MPI 1)

coolfluid_add_test( UTEST utest-ufem-buildsparsity
                    CPP utest-ufem-buildsparsity.cpp
                    LIBS coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem coolfluid_mesh_blockmesh
                    MPI 1)

coolfluid_add_test( UTEST utest-scalar-advection
                    CPP utest-scalar-advection.cpp
                    LIBS coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem
                    MPI 1)

coolfluid_add_test( UTEST utest-rdm-merge
                    CPP utest-rdm-merge.cpp
                    LIBS coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem coolfluid_mesh_blockmesh                    
                    MPI 1)

coolfluid_add_test( UTEST utest-proto-ns-assembly
                    CPP utest-proto-ns-assembly.cpp
                    LIBS coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_solver coolfluid_ufem
                    MPI 1)

coolfluid_add_test( UTEST utest-neumann
                    CPP utest-neumann.cpp
                    LIBS coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem
                    MPI 1)

# Disable debugging on the compiled expressions, since this takes huge amounts of memory
set_source_files_properties(NavierStokes.cpp PROPERTIES COMPILE_FLAGS "-g0")

coolfluid_add_test( ATEST atest-ufem-heat1d-P2
                    PYTHON atest-ufem-heat1d-P2.py)

coolfluid_add_test( ATEST atest-ufem-heat2d-disk
                    PYTHON atest-ufem-heat2d-disk.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/ring2d-quads.neu )

coolfluid_add_test( ATEST atest-ufem-heat3d-channel
                    PYTHON atest-ufem-heat3d-channel.py
                    MPI 4 )

coolfluid_add_test( ATEST atest-ufem-heat3d-sphere
                    PYTHON atest-ufem-heat3d-sphere.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/ring3d-tetras.neu
                    MPI 4 )

coolfluid_add_test( ATEST atest-ufem-navier-stokes-cylinder2d
                    PYTHON atest-ufem-navier-stokes-cylinder2d.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/kvs15.neu
                    MPI 1)

coolfluid_add_test( ATEST atest-ufem-navier-stokes-laminar-channel-2d
                    PYTHON atest-ufem-navier-stokes-laminar-channel-2d.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/kvs15.neu
                    MPI 2)

coolfluid_add_test( ATEST atest-ufem-navier-stokes-explicit-laminar-channel-2d
                    PYTHON atest-ufem-navier-stokes-explicit-laminar-channel-2d.py
                    MPI 1)

coolfluid_add_test( ATEST atest-ufem-navier-stokes-explicit-implicit-laminar-channel-2d
                    PYTHON atest-ufem-navier-stokes-explicit-implicit-laminar-channel-2d.py
                    MPI 1)

coolfluid_add_test( ATEST atest-quadtriag
                    PYTHON atest-quadtriag.py
                    ARGUMENTS ${CMAKE_SOURCE_DIR}/resources/quadtriag.neu)

coolfluid_add_test( ATEST atest-flatplate2d-laminar
                    PYTHON atest-flatplate2d-laminar.py)

coolfluid_add_test( ATEST atest-flatplate2d-scalar
                    PYTHON atest-flatplate2d-scalar.py)

coolfluid_add_test( ATEST atest-flatplate2d-scalar-neumann
                    PYTHON atest-flatplate2d-scalar-neumann.py)

coolfluid_add_test( ATEST atest-flatplate2d-spalartallmaras
                    PYTHON atest-flatplate2d-spalartallmaras.py)

coolfluid_add_test( ATEST atest-multiregion
                    PYTHON atest-multiregion.py)

coolfluid_add_test( ATEST atest-conjugate-heat-flatplate
                    PYTHON atest-conjugate-heat-flatplate.py)

coolfluid_add_test( ATEST atest-boussinesq
                    PYTHON atest-boussinesq.py)

coolfluid_add_test( PTEST ptest-navier-stokes-assembly
                    PYTHON ptest-navier-stokes-assembly.py)
