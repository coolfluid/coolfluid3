include_directories(${Trilinos_INCLUDE_DIRS})
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Trilinos_CXX_COMPILER_FLAGS}" )
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

coolfluid_add_test( ATEST utest-ufem-heat2d-periodic
                    PYTHON utest-ufem-heat2d-periodic.py
                    MPI 4 )

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
                    MPI 4)

coolfluid_add_test( ATEST atest-ufem-navier-stokes-laminar-channel-2d
                    PYTHON atest-ufem-navier-stokes-laminar-channel-2d.py
                    MPI 2)

coolfluid_add_test( ATEST atest-ufem-navier-stokes-periodic-channel-2d
                    PYTHON atest-ufem-navier-stokes-periodic-channel-2d.py
                    MPI 2)

coolfluid_add_test( ATEST atest-ufem-ns-driven-cavity
                    PYTHON atest-ufem-ns-driven-cavity.py
                    MPI 1)

coolfluid_add_test( ATEST atest-ufem-navier-stokes-taylor-green
                    PYTHON atest-ufem-navier-stokes-taylor-green.py
                    MPI 4
                    ARGUMENTS --dt=0.004 --elem=quad --segs=32 --theta=0.5 --tsteps=5)

coolfluid_add_test( ATEST atest-ufem-navier-stokes-restart
                    PYTHON atest-ufem-navier-stokes-restart.py
                    MPI 2)

coolfluid_add_test( ATEST atest-ufem-navier-stokes-semi-implicit-laminar-channel-2d
                    PYTHON atest-ufem-navier-stokes-semi-implicit-laminar-channel-2d.py
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

coolfluid_add_test( ATEST atest-flatplate2d-kepsilon
                    PYTHON atest-flatplate2d-kepsilon.py)

coolfluid_add_test( ATEST atest-channel-chienkepsilon
                    PYTHON atest-channel-chienkepsilon.py
                    MPI 4)

coolfluid_add_test( ATEST atest-channel-sst
                    PYTHON atest-channel-sst.py
                    MPI 4)

coolfluid_add_test( ATEST atest-channel-standardkepsilon
                    PYTHON atest-channel-standardkepsilon.py)

coolfluid_add_test( ATEST atest-comte-bellot
                    PYTHON atest-comte-bellot.py)

coolfluid_add_test( ATEST atest-kepsilon-grid
                    PYTHON atest-kepsilon-grid.py)

coolfluid_add_test( ATEST atest-channel-spalart-allmaras
                    PYTHON atest-channel-spalart-allmaras.py)

coolfluid_add_test( ATEST atest-multiregion
                    PYTHON atest-multiregion.py)

coolfluid_add_test( ATEST atest-conjugate-heat-flatplate
                    PYTHON atest-conjugate-heat-flatplate.py)

coolfluid_add_test( ATEST atest-boussinesq
                    PYTHON atest-boussinesq.py)

coolfluid_add_test( PTEST ptest-navier-stokes-assembly
                    PYTHON ptest-navier-stokes-assembly.py)

coolfluid_add_test( UTEST utest-ufem-teko-blocks
                    CPP utest-ufem-teko-blocks.cpp
                    LIBS coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem coolfluid_mesh_blockmesh
                    MPI 1)

coolfluid_add_test(UTEST utest-ufem-surfaceintegral
                   PYTHON utest-ufem-surfaceintegral.py
                   MPI 4)

coolfluid_add_test(UTEST utest-ufem-volumeintegral
                   PYTHON utest-ufem-volumeintegral.py
                   MPI 4)

coolfluid_add_test( ATEST atest-ufem-navier-stokes-mlaux
                    PYTHON atest-ufem-navier-stokes-mlaux.py
                    MPI 2)

coolfluid_add_test( ATEST atest-ufem-velocitygradient
                    PYTHON atest-ufem-velocitygradient.py
                    MPI 4 )

coolfluid_add_test( ATEST atest-ufem-pressurenormalization
                    PYTHON atest-ufem-pressurenormalization.py
                    MPI 4 )

coolfluid_add_test( ATEST utest-kepsilonphysics
                    PYTHON utest-kepsilonphysics.py)

coolfluid_add_test( ATEST atest-ufem-abl
                    PYTHON atest-ufem-abl.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/abl.msh
                    MPI 4)

coolfluid_add_test( ATEST atest-ufem-abl-boussinesqconcentration
                    PYTHON atest-ufem-abl-boussinesqconcentration.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/abl.msh
                    MPI 4)

coolfluid_add_test( ATEST atest-unsteady-bodyforce
                    PYTHON atest-unsteady-bodyforce.py)

coolfluid_add_test(UTEST utest-ufem-bcfeedback
                   PYTHON utest-ufem-bcfeedback.py
                   MPI 4)

add_subdirectory(demo)
add_subdirectory(les)
add_subdirectory(particles)
add_subdirectory(adjoint)
add_subdirectory(adjointtube)
add_subdirectory(compressible)
add_subdirectory(meshes)
