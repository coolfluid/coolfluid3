option(CF3_UFEM_ENABLE_TRIAGS "Enable code generation for triangular elements, for solvers provide compile-time element selection"      ON)
option(CF3_UFEM_ENABLE_QUADS  "Enable code generation for quadrilateral elements, for solvers provide compile-time element selection"   ON)
option(CF3_UFEM_ENABLE_TETRAS "Enable code generation for tetrahedral elements, for solvers provide compile-time element selection"     ON)
option(CF3_UFEM_ENABLE_HEXAS  "Enable code generation for hexahedral elements, for solvers provide compile-time element selection"      ON)

mark_as_advanced(CF3_UFEM_ENABLE_TRIAGS)
mark_as_advanced(CF3_UFEM_ENABLE_QUADS)
mark_as_advanced(CF3_UFEM_ENABLE_TETRAS)
mark_as_advanced(CF3_UFEM_ENABLE_HEXAS)

add_subdirectory(config)
add_subdirectory(ns_implementation)

list( APPEND coolfluid_ufem_files
  AdjacentCellToFace.hpp
  AdjacentCellToFace.cpp
  BCHoldValue.hpp
  BCHoldValue.cpp
  BCNeumannConstant.cpp
  BCNeumannConstant.hpp
  BoundaryConditions.hpp
  BoundaryConditions.cpp
  BoussinesqAssembly.hpp
  BoussinesqAssemblyExtended.hpp
  ComputeCFL.hpp
  ComputeCFL.cpp
  CouplingIterationControl.cpp
  CouplingIterationControl.hpp
  CriterionConvergence.cpp
  CriterionConvergence.hpp
  HeatConductionManual.hpp
  HeatConductionManual.cpp
  HeatConductionSteady.hpp
  HeatConductionSteady.cpp
  HeatConductionVirtual.hpp
  HeatConductionVirtual.cpp
  HeatCouplingFlux.hpp
  HeatCouplingFlux.cpp
  HeatCouplingRobin.hpp
  HeatCouplingRobin.cpp
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
  StokesSteady.hpp
  StokesSteady.cpp
  SurfaceIntegral.hpp
  SurfaceIntegral.cpp
  SUPG.hpp
  Tags.hpp
  Tags.cpp
)

list( APPEND coolfluid_ufem_cflibs coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_solver_actions coolfluid_physics coolfluid_mesh)
set(coolfluid_ufem_RCC $<TARGET_OBJECTS:cf3_ufem_ns_implementation>)
set( coolfluid_ufem_condition ${CF3_ENABLE_PROTO} )

coolfluid_add_library( coolfluid_ufem )
set_source_files_properties(
  HeatConductionSteady.cpp
  SpalartAllmaras.cpp
PROPERTIES COMPILE_FLAGS "-g0")
