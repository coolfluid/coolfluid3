list( APPEND coolfluid_fvm_files
  LibFVM.cpp
  LibFVM.hpp
)

list( APPEND coolfluid_fvm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_solver_actions coolfluid_solver )

coolfluid_add_library( coolfluid_fvm )
