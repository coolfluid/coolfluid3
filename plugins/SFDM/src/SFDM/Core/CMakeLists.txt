list( APPEND coolfluid_sfdm_core_files
  CreateSpace.hpp
  CreateSpace.cpp
  LibCore.hpp
  LibCore.cpp
)

list( APPEND coolfluid_sfdm_core_cflibs coolfluid_mesh coolfluid_mesh_sf )

coolfluid_add_library( coolfluid_sfdm_core )

