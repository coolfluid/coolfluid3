list( APPEND coolfluid_sfdm_core_files
  CreateSpace.hpp
  CreateSpace.cpp
  LibCore.hpp
  LibCore.cpp
  ShapeFunction.hpp
  ShapeFunction.cpp
  SFDSolver.hpp
  SFDSolver.cpp
)

list( APPEND coolfluid_sfdm_core_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_mesh_actions coolfluid_solver coolfluid_solver_actions)

coolfluid_add_library( coolfluid_sfdm_core )

