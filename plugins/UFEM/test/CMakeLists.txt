# UFEM tests

coolfluid_add_test( UTEST      utest-proto-heat
                    CPP        utest-proto-heat.cpp
                    ARGUMENTS  ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                    LIBS       coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem
                    MPI        1
                    CONDITION  coolfluid_ufem_builds )

coolfluid_add_test( UTEST      utest-proto-heat-parallel
                    CPP        utest-proto-heat-parallel.cpp
                    ARGUMENTS  ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                    LIBS       coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem coolfluid_mesh_blockmesh
                    MPI        4
                    CONDITION  coolfluid_ufem_builds )

coolfluid_add_test( UTEST      utest-proto-unsteady
                    CPP        utest-proto-unsteady.cpp
                    ARGUMENTS  ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                    LIBS       coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem
                    MPI        1
                    CONDITION  coolfluid_ufem_builds )

coolfluid_add_test( UTEST      utest-proto-systems
                    CPP        utest-proto-systems.cpp
                    ARGUMENTS  ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                    LIBS       coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem
                    MPI        1
                    CONDITION  coolfluid_ufem_builds )

coolfluid_add_test( UTEST      utest-proto-navier-stokes
                    CPP        utest-proto-navier-stokes.cpp NavierStokes.hpp NavierStokes.cpp
                    ARGUMENTS  ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                    LIBS       coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem
                    MPI        1
                    CONDITION  coolfluid_ufem_builds )

coolfluid_add_test( UTEST      utest-ufem-buildsparsity
                    CPP        utest-ufem-buildsparsity.cpp
                    LIBS       coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem coolfluid_mesh_blockmesh
                    MPI        1
                    CONDITION  coolfluid_ufem_builds )

# Disable debugging on the compiled expressions, since this takes huge amounts of memory
set_source_files_properties(NavierStokes.cpp PROPERTIES COMPILE_FLAGS "-g0")

set( UFEM_LSS_CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )
set( UFEM_HEAT_INPUT_MESH ${CMAKE_CURRENT_SOURCE_DIR}/meshes/ring2d-quads.neu )

coolfluid_add_test( ATEST     atest-ufem-heat2d-disk
                    PYTHON    atest-ufem-heat2d-disk.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/ring2d-quads.neu ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml )

coolfluid_add_test( ATEST     atest-ufem-heat3d-channel
                    PYTHON    atest-ufem-heat3d-channel.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                    MPI       4 )

coolfluid_add_test( ATEST     atest-ufem-heat3d-sphere
                    PYTHON    atest-ufem-heat3d-sphere.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/ring3d-tetras.neu ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                    MPI       4 )

coolfluid_add_test( ATEST     atest-ufem-navier-stokes-cylinder2d
                    PYTHON    atest-ufem-navier-stokes-cylinder2d.py
                    ARGUMENTS ${CMAKE_CURRENT_SOURCE_DIR}/meshes/kvs15.neu ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                    MPI       4)

coolfluid_add_test( UTEST      utest-rdm-merge
                    CPP        utest-rdm-merge.cpp
                    LIBS       coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_ufem coolfluid_mesh_blockmesh
                    ARGUMENTS  ${CMAKE_CURRENT_SOURCE_DIR}/solver.xml
                    MPI        1
                    CONDITION  coolfluid_ufem_builds )

coolfluid_add_test( UTEST      utest-proto-ns-assembly
                    CPP        utest-proto-ns-assembly.cpp
                    LIBS       coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_solver
                    CONDITION  CF3_ENABLE_PROTO )
