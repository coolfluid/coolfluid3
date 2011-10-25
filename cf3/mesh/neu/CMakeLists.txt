list( APPEND coolfluid_mesh_neu_files
  Reader.hpp
  Reader.cpp
  Writer.hpp
  Writer.cpp
  LibNeu.cpp
  LibNeu.hpp
  Shared.hpp
  Shared.cpp
)

list( APPEND coolfluid_mesh_neu_cflibs coolfluid_mesh_lagrangep1)

set( coolfluid_mesh_neu_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_neu )
