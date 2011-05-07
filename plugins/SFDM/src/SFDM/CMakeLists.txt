list( APPEND coolfluid_sfdm_files
  ComputeJacobianDeterminant.hpp
  ComputeJacobianDeterminant.cpp
  ComputeRhsInCell.hpp
  ComputeRhsInCell.cpp
  CreateSpace.hpp
  CreateSpace.cpp
  LibSFDM.hpp
  LibSFDM.cpp
  Reconstruct.hpp
  Reconstruct.cpp
  Flux.hpp
  Flux.cpp
  ShapeFunction.hpp
  ShapeFunction.cpp
  SFDSolver.hpp
  SFDSolver.cpp
)

list( APPEND coolfluid_sfdm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_mesh_actions coolfluid_solver coolfluid_solver_actions)

coolfluid_add_library( coolfluid_sfdm )


add_subdirectory( SF )       # coolfluid_sfdm_sf library
