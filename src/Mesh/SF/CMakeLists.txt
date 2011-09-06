list( APPEND coolfluid_mesh_sf_files
  LibSF.cpp
  LibSF.hpp
  Types.hpp
)

list( APPEND coolfluid_mesh_sf_cflibs coolfluid_mesh )

set( coolfluid_mesh_sf_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_sf )
