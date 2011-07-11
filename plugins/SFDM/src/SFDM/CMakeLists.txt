# TQ: NOTE
#     I leave the sources here but deactivate this code because
#     it follows the old Physics API.
#     The owner of this module should upgrade it to the new API or remove it.

list( APPEND coolfluid_sfdm_condition FALSE )

list( APPEND coolfluid_sfdm_files
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
  LibSFDM.hpp
  LibSFDM.cpp
  OutputIterationInfo.hpp
  OutputIterationInfo.cpp
  Reconstruct.hpp
  Reconstruct.cpp
  ShapeFunction.hpp
  ShapeFunction.cpp
  SFDSolver.hpp
  SFDSolver.cpp
  SFDWizard.hpp
  SFDWizard.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp
)

list( APPEND coolfluid_sfdm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_mesh_actions coolfluid_solver coolfluid_solver_actions coolfluid_riemannsolvers coolfluid_rungekutta)

coolfluid_add_library( coolfluid_sfdm )


add_subdirectory( SF )       # coolfluid_sfdm_sf library
