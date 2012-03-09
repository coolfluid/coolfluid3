list( APPEND coolfluid_ufem_files
  BoundaryConditions.hpp
  BoundaryConditions.cpp
  HeatConductionSteady.hpp
  HeatConductionSteady.cpp
  LibUFEM.cpp
  LibUFEM.hpp
  LinearSolver.hpp
  LinearSolver.cpp
  LinearSolverUnsteady.hpp
  LinearSolverUnsteady.cpp
  NavierStokes.hpp
  NavierStokes.cpp
  NavierStokesSteady.hpp
  NavierStokesSteady.cpp
  NavierStokesOps.hpp
  ParsedFunctionExpression.hpp
  ParsedFunctionExpression.cpp
  SparsityBuilder.hpp
  SparsityBuilder.cpp
  Tags.hpp
  Tags.cpp
)

list( APPEND coolfluid_ufem_cflibs coolfluid_mesh_lagrangep1 coolfluid_solver_actions coolfluid_physics)

set( coolfluid_ufem_condition ${CF3_ENABLE_PROTO} )

coolfluid_add_library( coolfluid_ufem )
set_source_files_properties(HeatConductionSteady.cpp NavierStokes.cpp PROPERTIES COMPILE_FLAGS "-g0")
