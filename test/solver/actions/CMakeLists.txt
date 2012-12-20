################################################################################
# test Component Action

coolfluid_add_test( UTEST utest-solver-actions
                    CPP   utest-solver-actions.cpp DummyLoopOperation.hpp DummyLoopOperation.cpp
                    LIBS  coolfluid_solver_actions coolfluid_mesh_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_gmsh coolfluid_mesh_neu )
list( APPEND mesh_files  rotation-tg-p1.neu  rotation-qd-p1.neu  )
foreach( mfile ${mesh_files} )
  add_custom_command(TARGET utest-solver-actions
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CF3_RESOURCES_DIR}/${mfile} ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} )
endforeach()

################################################################################
# proto tests

if(CF3_ENABLE_PROTO)

if(CMAKE_BUILD_TYPE_CAPS MATCHES "RELEASE")
  set(_ARGS 16 16 120)
else()
  set(_ARGS 16 16 12)
endif()
coolfluid_add_test( PTEST      ptest-proto-benchmark
                    CPP        ptest-proto-benchmark.cpp
                    ARGUMENTS  ${_ARGS}
                    LIBS       coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_blockmesh coolfluid_testing coolfluid_mesh_generation coolfluid_solver)


coolfluid_add_test( UTEST     utest-proto-operators
                    CPP       utest-proto-operators.cpp
                    LIBS      coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_generation coolfluid_solver)
set_source_files_properties(utest-proto-operators.cpp PROPERTIES COMPILE_FLAGS "-g0")

coolfluid_add_test( UTEST     utest-proto-internals
                    CPP       utest-proto-internals.cpp
                    LIBS      coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_generation coolfluid_solver)


coolfluid_add_test( UTEST     utest-proto-components
                    CPP       utest-proto-components.cpp
                    LIBS      coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_generation coolfluid_solver)


coolfluid_add_test( UTEST     utest-proto-elements
                    CPP       utest-proto-elements.cpp
                    LIBS      coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_generation coolfluid_solver coolfluid_mesh_blockmesh)


if(CMAKE_BUILD_TYPE_CAPS MATCHES "RELEASE")
  set(_ARGS 160 160 120)
else()
  set(_ARGS 16 16 12)
endif()
coolfluid_add_test( PTEST     ptest-proto-parallel
                    CPP       ptest-proto-parallel.cpp
                    ARGUMENTS ${_ARGS}
                    LIBS      coolfluid_mesh coolfluid_solver_actions coolfluid_mesh_lagrangep1 coolfluid_mesh_blockmesh coolfluid_testing coolfluid_mesh_generation coolfluid_solver
                    MPI       4)
else()
coolfluid_mark_not_orphan(
  ptest-proto-benchmark.cpp
  utest-proto-operators.cpp
  utest-proto-internals.cpp
  utest-proto-components.cpp
  utest-proto-elements.cpp
  ptest-proto-parallel.cpp
)
endif()