add_subdirectory(ns_implementation)

list( APPEND coolfluid_ufem_files
  AdjacentCellToFace.hpp
  AdjacentCellToFace.cpp
  BCHoldValue.hpp
  BCHoldValue.cpp
  BoundaryConditions.hpp
  BoundaryConditions.cpp
  ComputeCFL.hpp
  ComputeCFL.cpp
  HeatConductionSteady.hpp
  HeatConductionSteady.cpp
  HeatCouplingFlux.hpp
  HeatCouplingFlux.cpp
  InitialConditionConstant.hpp
  InitialConditionConstant.cpp
  InitialConditionFunction.hpp
  InitialConditionFunction.cpp
  InitialConditions.hpp
  InitialConditions.cpp
  LibUFEM.cpp
  LibUFEM.hpp
  LSSAction.hpp
  LSSAction.cpp
  LSSActionUnsteady.hpp
  LSSActionUnsteady.cpp
  Solver.hpp
  Solver.cpp
  NavierStokes.hpp
  NavierStokesAssembly.hpp
  NavierStokesExplicit.hpp
  NavierStokesExplicitAssembly.hpp
  NavierStokesSpecializations.hpp
  NavierStokesPhysics.hpp
  NavierStokesPhysics.cpp
  ParsedFunctionExpression.hpp
  ParsedFunctionExpression.cpp
  ScalarAdvection.cpp
  ScalarAdvection.hpp
  SpalartAllmaras.cpp
  SpalartAllmaras.hpp
  SparsityBuilder.hpp
  SparsityBuilder.cpp
  SUPG.hpp
  Tags.hpp
  Tags.cpp
)

list( APPEND coolfluid_ufem_cflibs coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_solver_actions coolfluid_physics)
set(coolfluid_ufem_RCC $<TARGET_OBJECTS:cf3_ufem_ns_implementation>)
set( coolfluid_ufem_condition ${CF3_ENABLE_PROTO} )

coolfluid_add_library( coolfluid_ufem )
set_source_files_properties(
  HeatConductionSteady.cpp
  SpalartAllmaras.cpp
PROPERTIES COMPILE_FLAGS "-g0")
