list( APPEND coolfluid_neutral_files
  CReader.hpp
  CReader.cpp
  CWriter.hpp
  CWriter.cpp
  LibNeu.cpp
  LibNeu.hpp
  Shared.hpp
  Shared.cpp
)

list( APPEND coolfluid_neutral_cflibs coolfluid_mesh_sf)

set( coolfluid_neutral_kernellib TRUE )

coolfluid_add_library( coolfluid_neutral )
