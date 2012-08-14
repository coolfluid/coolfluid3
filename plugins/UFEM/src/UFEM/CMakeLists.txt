list( APPEND coolfluid_ufem_files
  AdjacentCellToFace.hpp
  AdjacentCellToFace.cpp
  BCHoldValue.hpp
  BCHoldValue.cpp
  BoundaryConditions.hpp
  BoundaryConditions.cpp
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
  NavierStokes.cpp
  NavierStokesOps.hpp
  NavierStokesOps.cpp
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
  Tags.hpp
  Tags.cpp
)

list( APPEND coolfluid_ufem_cflibs coolfluid_mesh_lagrangep1 coolfluid_mesh_lagrangep2 coolfluid_solver_actions coolfluid_physics)

set( coolfluid_ufem_condition ${CF3_ENABLE_PROTO} )

coolfluid_add_library( coolfluid_ufem )
set_source_files_properties(HeatConductionSteady.cpp NavierStokes.cpp NavierStokesOps.cpp SpalartAllmaras.cpp PROPERTIES COMPILE_FLAGS "-g0")
