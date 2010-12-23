list( APPEND coolfluid_rdm_files
  LibRDM.cpp
  LibRDM.hpp
)

list( APPEND coolfluid_rdm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_actions coolfluid_solver )

coolfluid_add_library( coolfluid_rdm )
