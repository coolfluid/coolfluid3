include_directories(${Trilinos_INCLUDE_DIRS})

option(CF3_UFEM_ENABLE_TRIAGS "Enable code generation for triangular elements, for solvers provide compile-time element selection"      ON)
option(CF3_UFEM_ENABLE_QUADS  "Enable code generation for quadrilateral elements, for solvers provide compile-time element selection"   ON)
option(CF3_UFEM_ENABLE_TETRAS "Enable code generation for tetrahedral elements, for solvers provide compile-time element selection"     ON)
option(CF3_UFEM_ENABLE_HEXAS  "Enable code generation for hexahedral elements, for solvers provide compile-time element selection"      ON)
option(CF3_UFEM_ENABLE_PRISMS "Enable code generation for prism (wedge) elements, for solvers provide compile-time element selection"   ON)

mark_as_advanced(CF3_UFEM_ENABLE_TRIAGS)
mark_as_advanced(CF3_UFEM_ENABLE_QUADS)
mark_as_advanced(CF3_UFEM_ENABLE_TETRAS)
mark_as_advanced(CF3_UFEM_ENABLE_HEXAS)

add_subdirectory(config)
add_subdirectory(ns_implementation)
add_subdirectory(ns_semi_implicit)

list( APPEND coolfluid_ufem_files
  AdjacentCellToFace.hpp
  AdjacentCellToFace.cpp
  AdjustForceTerm.hpp
  AdjustForceTerm.cpp
  BCDirichletFunction.hpp
  BCDirichletFunction.cpp
  BCFeedback.hpp
  BCFeedback.cpp
  BCHoldValue.hpp
  BCHoldValue.cpp
  BCNeumannConstant.cpp
  BCNeumannConstant.hpp
  BCSSTKOmega.hpp
  BCSSTKOmega.cpp
  BCWallEpsilon.hpp
  BCWallEpsilon.cpp
  BCWallEpsilonABL.hpp
  BCWallEpsilonABL.cpp
  BCWallFunctionABL.hpp
  BCWallFunctionABL.cpp
  BCWallFunctionABLGoit.hpp
  BCWallFunctionABLGoit.cpp
  BCWallFunctionABLGoitSI.hpp
  BCWallFunctionABLGoitSI.cpp
  BCWallFunctionNSImplicit.hpp
  BCWallFunctionNSImplicit.cpp
  BodyForceControl.cpp
  BodyForceControl.hpp
  BoundaryConditions.hpp
  BoundaryConditions.cpp
  BoussinesqConcentration.hpp
  BoussinesqConcentration.cpp
  BulkVelocity.hpp
  BulkVelocity.cpp
  ComputeCFL.hpp
  ComputeCFL.cpp
  CouplingIterationControl.cpp
  CouplingIterationControl.hpp
  CriterionConvergence.cpp
  CriterionConvergence.hpp
  CrossWindDiffusion.hpp
  HeatConductionSteady.hpp
  HeatConductionSteady.cpp
  HeatCouplingFlux.hpp
  HeatCouplingFlux.cpp
  ChienKEpsilon.hpp
  ChienKEpsilon.cpp
  ComputeFluxFluid.hpp
  ComputeFluxFluid.cpp
  ComputeFluxSolid.hpp
  ComputeFluxSolid.cpp
  EersteStap.hpp
  EersteStap.cpp
  GradPressureGradient.hpp
  GradPressureGradient.cpp
  HeatCouplingRobin.hpp
  HeatCouplingRobin.cpp
  HeatCouplingRobinFluid.hpp
  HeatCouplingRobinFluid.cpp
  ICNSImplicitToSemi.hpp
  ICNSImplicitToSemi.cpp
  ICNSSemiToImplicit.hpp
  ICNSSemiToImplicit.cpp
  InitialConditionConstant.hpp
  InitialConditionConstant.cpp
  InitialConditionFunction.hpp
  InitialConditionFunction.cpp
  InitialConditions.hpp
  InitialConditions.cpp
  KEpsilonBase.hpp
  KEpsilonBase.cpp
  KEpsilonPhysics.hpp
  KEpsilonPhysics.cpp
  KineticEnergyIntegral.hpp
  KineticEnergyIntegral.cpp
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
  NavierStokesSpecializations.hpp
  NavierStokesPhysics.hpp
  NavierStokesPhysics.cpp
  NormalizedNormals.hpp
  NormalizedNormals.cpp
  NSResidual.hpp
  NSResidual.cpp
  ParsedFunctionExpression.hpp
  ParsedFunctionExpression.cpp
  PressureGradient.hpp
  PressureGradient.cpp
  PressureNormalization.hpp
  PressureNormalization.cpp
  ScalarAdvection.cpp
  ScalarAdvection.hpp
  ScalarAdvectionCoupling.cpp
  ScalarAdvectionCoupling.hpp
  SpalartAllmaras.cpp
  SpalartAllmaras.hpp
  SparsityBuilder.hpp
  SparsityBuilder.cpp
  SSTKOmega.hpp
  SSTKOmega.cpp
  StandardKEpsilon.hpp
  StandardKEpsilon.cpp
  StokesSteady.hpp
  StokesSteady.cpp
  SurfaceIntegral.hpp
  SurfaceIntegral.cpp
  SUPG.hpp
  SUPG.cpp
  SUPGFields.hpp
  SUPGFields.cpp
  Tags.hpp
  Tags.cpp
  TweedeStap.hpp
  TweedeStap.cpp
  UnsteadyAction.hpp
  UnsteadyAction.cpp
  VelocityGradient.hpp
  VelocityGradient.cpp
  VolumeIntegral.hpp
  VolumeIntegral.cpp
  WriteRestartManager.hpp
  WriteRestartManager.cpp

  ns_semi_implicit/LSSVectorOps.hpp
  ns_semi_implicit/MatrixAssembly.hpp
  ns_semi_implicit/NavierStokesSemiImplicit.hpp
  ns_semi_implicit/PressureSystem.hpp
  ns_semi_implicit/PressureSystem.cpp

  TemperatureHistoryScalarAdvection.cpp
  TemperatureHistoryScalarAdvection.hpp
  TemperatureHistoryHeatConduction.cpp
  TemperatureHistoryHeatConduction.hpp
)

set(coolfluid_ufem_RCC $<TARGET_OBJECTS:cf3_ufem_ns_implementation> $<TARGET_OBJECTS:cf3_ufem_ns_semi_implicit_implementation>)

coolfluid3_add_library( TARGET     coolfluid_ufem
            SOURCES
              ${coolfluid_ufem_files}
            RCC
              ${coolfluid_ufem_RCC}
            LIBS
              coolfluid_mesh_lagrangep1
              coolfluid_mesh_lagrangep2
              coolfluid_solver_actions
              coolfluid_physics
              coolfluid_mesh
              CONDITION  CF3_ENABLE_PROTO )

set_source_files_properties(
  HeatConductionSteady.cpp
  NSResidual.cpp
  SpalartAllmaras.cpp
  SUPGFields.cpp
PROPERTIES COMPILE_FLAGS "-g0")

add_subdirectory(demo)
add_subdirectory(les)
add_subdirectory(particles)
add_subdirectory(adjoint)
add_subdirectory(adjointtube)
add_subdirectory(compressible)
