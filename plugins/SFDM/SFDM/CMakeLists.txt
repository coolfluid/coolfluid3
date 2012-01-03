list( APPEND coolfluid_sfdm_files
  BC.hpp
  BC.cpp
  BCConstant.hpp
  BCConstant.cpp
  BCFunction.hpp
  BCFunction.cpp
  BCDirichlet.hpp
  BCDirichlet.cpp
  BCStrong.hpp
  BCStrong.cpp
  BCWeak.hpp
  BCWeak.cpp
  BoundaryConditions.hpp
  BoundaryConditions.cpp
  ComputeUpdateCoefficient.hpp
  ComputeUpdateCoefficient.cpp
  Convection.hpp
  Convection.cpp
  ConvectiveTerm.hpp
  ConvectiveTerm.cpp
  CreateSFDFields.hpp
  CreateSFDFields.cpp
  DomainDiscretization.cpp
  DomainDiscretization.hpp
  ElementCaching.hpp
  Init.cpp
  Init.hpp
  InitialConditions.cpp
  InitialConditions.hpp
  IterativeSolver.cpp
  IterativeSolver.hpp
  LagrangeLocally1D.hpp
  LagrangeLocally1D.cpp
  LibSFDM.hpp
  LibSFDM.cpp
  Operations.hpp
  PrepareMesh.cpp
  PrepareMesh.hpp
  Reconstructions.hpp
  ShapeFunction.hpp
  ShapeFunction.cpp
  SFDSolver.hpp
  SFDSolver.cpp
  Tags.hpp
  Tags.cpp
  Term.cpp
  Term.hpp
  TimeStepping.cpp
  TimeStepping.hpp
  UpdateSolution.hpp
  UpdateSolution.cpp
)

list( APPEND coolfluid_sfdm_cflibs coolfluid_common coolfluid_mesh_actions coolfluid_solver coolfluid_solver_actions coolfluid_physics coolfluid_riemannsolvers)

coolfluid_add_library( coolfluid_sfdm )

coolfluid_mark_not_orphan(
)

add_subdirectory( scalar       )  # library coolfluid_sfdm_scalar
add_subdirectory( navierstokes )  # library coolfluid_sfdm_navierstokes
add_subdirectory( lineuler     )  # library coolfluid_sfdm_lineuler
