list( APPEND coolfluid_mesh_neutral_files
  CReader.hpp
  CReader.cpp
  CWriter.hpp
  CWriter.cpp
  LibNeu.cpp
  LibNeu.hpp
  Shared.hpp
  Shared.cpp
)

list( APPEND coolfluid_mesh_neutral_cflibs coolfluid_mesh_sf)

set( coolfluid_mesh_neutral_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_neutral )
