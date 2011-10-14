################################################################################
# test Component CAction

coolfluid_add_test( UTEST utest-solver-actions
                    CPP   utest-solver-actions.cpp CDummyLoopOperation.hpp CDummyLoopOperation.cpp
                    LIBS  coolfluid_solver_actions coolfluid_mesh_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_gmsh coolfluid_mesh_neu )
list( APPEND mesh_files  rotation-tg-p1.neu  rotation-qd-p1.neu  )
foreach( mfile ${mesh_files} )
  add_custom_command(TARGET utest-solver-actions
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF_RESOURCE_DIR}/${mfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} )
endforeach()



coolfluid_add_test( UTEST utest-solver-solvesystem
                    CPP   utest-solver-solvesystem.cpp
                    LIBS  coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_mesh_lagrangep3 coolfluid_mesh_generation coolfluid_solver coolfluid_math_lss
                    MPI   1 )

################################################################################
# proto tests

if(CMAKE_BUILD_TYPE_CAPS MATCHES "RELEASE")
  set(_ARGS 160 160 120)
else()
  set(_ARGS 16 16 12)
endif()
coolfluid_add_test( UTEST      utest-proto-benchmark
                    CPP        utest-proto-benchmark.cpp
                    ARGUMENTS  ${_ARGS}
                    LIBS       coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_blockmesh coolfluid_testing coolfluid_mesh_generation coolfluid_solver
                    CONDITION  CF_ENABLE_PROTO )


coolfluid_add_test( UTEST     utest-proto-operators
                    CPP       utest-proto-operators.cpp
                    LIBS      coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_generation coolfluid_solver
                    CONDITION CF_ENABLE_PROTO )


coolfluid_add_test( UTEST     utest-proto-components
                    CPP       utest-proto-components.cpp
                    LIBS      coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_generation coolfluid_solver
                    CONDITION CF_ENABLE_PROTO )


coolfluid_add_test( UTEST     utest-proto-elements
                    CPP       utest-proto-elements.cpp
                    LIBS      coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_generation coolfluid_solver
                    CONDITION CF_ENABLE_PROTO )


if(CMAKE_BUILD_TYPE_CAPS MATCHES "RELEASE")
  set(_ARGS 160 160 120)
else()
  set(_ARGS 16 16 12)
endif()
coolfluid_add_test( UTEST     utest-proto-parallel
                    CPP       utest-proto-parallel.cpp
                    ARGUMENTS ${_ARGS}
                    LIBS      coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_blockmesh coolfluid_testing coolfluid_mesh_generation coolfluid_solver
                    MPI       4
                    CONDITION CF_ENABLE_PROTO )
