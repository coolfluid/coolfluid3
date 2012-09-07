list( APPEND coolfluid_sdm_files
  BC.hpp
  BC.cpp
  BCConstant.hpp
  BCConstant.cpp
  BCExtrapolate.hpp
  BCExtrapolate.cpp
  BCFunction.hpp
  BCFunction.cpp
  BCWeak.hpp
  BCWeak.cpp
  BCStrong.hpp
  BCStrong.cpp
  BoundaryConditions.hpp
  BoundaryConditions.cpp
  ComputeLNorm.hpp
  ComputeLNorm.cpp
  ComputeCellJacobianPerturb.hpp
  ComputeCellJacobianPerturb.cpp
  ImposeCFL.hpp
  ImposeCFL.cpp
  ConvectiveTerm.hpp
  ConvectiveTerm.cpp
  Solver.hpp
  Solver.cpp
  DiffusiveTerm.hpp
  DiffusiveTerm.cpp
  CreateSDFields.hpp
  CreateSDFields.cpp
  DomainDiscretization.cpp
  DomainDiscretization.hpp
  ElementCaching.hpp
  ElementCaching.cpp
  ExplicitSolver.hpp
  ExplicitSolver.cpp
  Init.cpp
  Init.hpp
  InitialConditions.cpp
  InitialConditions.hpp
  IterativeSolver.cpp
  IterativeSolver.hpp
  PhysDataBase.hpp
  ExplicitRungeKuttaLowStorage2.hpp
  ExplicitRungeKuttaLowStorage2.cpp
  ExplicitRungeKuttaLowStorage3.hpp
  ExplicitRungeKuttaLowStorage3.cpp
  LagrangeLocally1D.hpp
  LagrangeLocally1D.cpp
  LibSDM.hpp
  LibSDM.cpp
  Operations.hpp
  PrepareMesh.cpp
  PrepareMesh.hpp
  Reconstructions.hpp
  SDSolver.hpp
  SDSolver.cpp
  Model.hpp
  Model.cpp
  SourceTerm.hpp
  SourceTerm.cpp
  ShapeFunction.hpp
  ShapeFunction.cpp
  SourceTimeInterpolation2D.hpp
  SourceTimeInterpolation2D.cpp
  System.hpp
  System.cpp
  Tags.hpp
  Tags.cpp
  Term.cpp
  Term.hpp
  TimeStepping.cpp
  TimeStepping.hpp
  TimeIntegrationStepComputer.hpp
  TimeIntegrationStepComputer.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp
)

list( APPEND coolfluid_sdm_cflibs coolfluid_common coolfluid_mesh_actions coolfluid_solver coolfluid_solver_actions coolfluid_physics coolfluid_riemannsolvers coolfluid_mesh_lagrangep2)

coolfluid_add_library( coolfluid_sdm )

coolfluid_mark_not_orphan(
)

add_subdirectory( scalar       )                 # library coolfluid_sdm_scalar
add_subdirectory( navierstokes )                 # library coolfluid_sdm_navierstokes
add_subdirectory( navierstokesmovingreference )  # library coolfluid_sdm_navierstokesmovingreference
add_subdirectory( lineuler     )                 # library coolfluid_sdm_lineuler
add_subdirectory( explicit_rungekutta )          # library coolfluid_sdm_explicit_rungekutta
add_subdirectory( lusgs )                        # library coolfluid_sdm_lusgs
add_subdirectory( implicit )                     # library coolfluid_sdm_implicit

