list( APPEND coolfluid_fvm_sf_files
  LibSF.cpp
  LibSF.hpp
)

list( APPEND coolfluid_fvm_sf_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_solver_actions coolfluid_solver coolfluid_mesh_actions )

coolfluid_add_library( coolfluid_fvm_sf )

