list( APPEND coolfluid_sdm_files
  BC.hpp
  BC.cpp
  BCConstant.hpp
  BCConstant.cpp
  BCFunction.hpp
  BCFunction.cpp
  BCWeak.hpp
  BCWeak.cpp
  BoundaryConditions.hpp
  BoundaryConditions.cpp
  ComputeUpdateCoefficient.hpp
  ComputeUpdateCoefficient.cpp
  ConvectiveTerm.hpp
  ConvectiveTerm.cpp
  CreateSDFields.hpp
  CreateSDFields.cpp
  DomainDiscretization.cpp
  DomainDiscretization.hpp
  ElementCaching.hpp
  Init.cpp
  Init.hpp
  InitialConditions.cpp
  InitialConditions.hpp
  IterativeSolver.cpp
  IterativeSolver.hpp
  PhysDataBase.hpp
  RungeKuttaLowStorage2.hpp
  RungeKuttaLowStorage2.cpp
  RungeKuttaLowStorage3.hpp
  RungeKuttaLowStorage3.cpp
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
  SourceTerm.hpp
  SourceTerm.cpp
  ShapeFunction.hpp
  ShapeFunction.cpp
  Tags.hpp
  Tags.cpp
  Term.cpp
  Term.hpp
  TimeStepping.cpp
  TimeStepping.hpp
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
