list( APPEND coolfluid_sfdm_common_files
  LibCommon.hpp
  LibCommon.cpp
)

list( APPEND coolfluid_sfdm_common_cflibs coolfluid_mesh coolfluid_mesh_sf )

coolfluid_add_library( coolfluid_sfdm_common )

