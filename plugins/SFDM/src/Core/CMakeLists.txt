list( APPEND coolfluid_sfdm_core_files
  LibCore.hpp
  LibCore.cpp
)

list( APPEND coolfluid_sfdm_core_cflibs coolfluid_mesh coolfluid_mesh_sf )

coolfluid_add_library( coolfluid_sfdm_core )

