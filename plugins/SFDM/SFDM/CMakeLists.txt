list( APPEND coolfluid_sfdm_files
  BCDirichlet.hpp
  BCDirichlet.cpp
  ComputeUpdateCoefficient.hpp
  ComputeUpdateCoefficient.cpp
  Convection.hpp
  Convection.cpp
  CreateSFDFields.hpp
  CreateSFDFields.cpp
  DomainDiscretization.cpp
  DomainDiscretization.hpp
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
  PrepareMesh.cpp
  PrepareMesh.hpp
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
