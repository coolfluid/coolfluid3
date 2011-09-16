list( APPEND coolfluid_sfdm_files
  #ComputeJacobianDeterminant.hpp
  #ComputeJacobianDeterminant.cpp
  #ComputeRhsInCell.hpp
  #ComputeRhsInCell.cpp
  #ComputeUpdateCoefficient.hpp
  #ComputeUpdateCoefficient.cpp
  #CreateSFDFields.hpp
  #CreateSFDFields.cpp
  #CreateSpace.hpp
  #CreateSpace.cpp
  CellTerm.cpp  CellTerm.hpp
  DomainDiscretization.cpp
  DomainDiscretization.hpp
  IterativeSolver.cpp  IterativeSolver.hpp
  LibSFDM.hpp
  LibSFDM.cpp
  #OutputIterationInfo.hpp
  #OutputIterationInfo.cpp
  #Reconstruct.hpp
  #Reconstruct.cpp
  #ShapeFunction.hpp
  #ShapeFunction.cpp
  SFDSolver.hpp
  SFDSolver.cpp
  TimeStepping.cpp  TimeStepping.hpp
  #SFDWizard.hpp
  #SFDWizard.cpp
  #UpdateSolution.hpp
  #UpdateSolution.cpp
)

list( APPEND coolfluid_sfdm_cflibs coolfluid_mesh coolfluid_solver coolfluid_solver_actions coolfluid_physics)

coolfluid_add_library( coolfluid_sfdm )

coolfluid_mark_not_orphan(
  ComputeJacobianDeterminant.hpp
  ComputeJacobianDeterminant.cpp
  ComputeRhsInCell.hpp
  ComputeRhsInCell.cpp
  ComputeUpdateCoefficient.hpp
  ComputeUpdateCoefficient.cpp
  CreateSFDFields.hpp
  CreateSFDFields.cpp
  CreateSpace.hpp
  CreateSpace.cpp
  OutputIterationInfo.hpp
  OutputIterationInfo.cpp
  Reconstruct.hpp
  Reconstruct.cpp
  ShapeFunction.hpp
  ShapeFunction.cpp
  SFDWizard.hpp
  SFDWizard.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp
)

add_subdirectory( SF )       # coolfluid_sfdm_sf library
