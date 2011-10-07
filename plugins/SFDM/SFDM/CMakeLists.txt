list( APPEND coolfluid_sfdm_files
  #ComputeJacobianDeterminant.hpp
  #ComputeJacobianDeterminant.cpp
  #ComputeRhsInCell.hpp
  #ComputeRhsInCell.cpp
  ComputeUpdateCoefficient.hpp
  ComputeUpdateCoefficient.cpp
  Convection.hpp
  Convection.cpp
  CreateSFDFields.hpp
  CreateSFDFields.cpp
  #CreateSpace.hpp
  #CreateSpace.cpp
  DomainDiscretization.cpp
  DomainDiscretization.hpp
  Init.cpp
  Init.hpp
  InitialConditions.cpp
  InitialConditions.hpp
  IterativeSolver.cpp
  IterativeSolver.hpp
  LibSFDM.hpp
  LibSFDM.cpp
  PrepareMesh.cpp
  PrepareMesh.hpp
  #OutputIterationInfo.hpp
  #OutputIterationInfo.cpp
  #Reconstruct.hpp
  #Reconstruct.cpp
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
  #SFDWizard.hpp
  #SFDWizard.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp

  P0/Line.hpp
  P0/Line.cpp
  P0/Quad.hpp
  P0/Quad.cpp

  P1/Line.hpp
  P1/Line.cpp
  P1/Quad.hpp
  P1/Quad.cpp

  P2/Line.hpp
  P2/Line.cpp
  P2/Quad.hpp
  P2/Quad.cpp

  P3/Line.hpp
  P3/Line.cpp
)

list( APPEND coolfluid_sfdm_cflibs coolfluid_mesh_actions coolfluid_solver coolfluid_solver_actions coolfluid_physics coolfluid_riemannsolvers)

coolfluid_add_library( coolfluid_sfdm )

coolfluid_mark_not_orphan(
  ComputeJacobianDeterminant.hpp
  ComputeJacobianDeterminant.cpp
  ComputeRhsInCell.hpp
  ComputeRhsInCell.cpp
  CreateSpace.hpp
  CreateSpace.cpp
  OutputIterationInfo.hpp
  OutputIterationInfo.cpp
  Reconstruct.hpp
  Reconstruct.cpp
  SFDWizard.hpp
  SFDWizard.cpp
)
