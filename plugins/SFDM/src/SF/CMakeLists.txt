list( APPEND coolfluid_sfdm_sf_files
  LibSF.hpp
  LibSF.cpp
)

list( APPEND coolfluid_sfdm_sf_cflibs coolfluid_mesh coolfluid_mesh_sf )

coolfluid_add_library( coolfluid_sfdm_sf )

