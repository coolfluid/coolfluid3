list( APPEND coolfluid_mesh_gmsh_files
  Writer.hpp
  Writer.cpp
  Reader.hpp
  Reader.cpp
  LibGmsh.cpp
  LibGmsh.hpp
  Shared.cpp
  Shared.hpp
)

list( APPEND coolfluid_mesh_gmsh_cflibs coolfluid_mesh )

set( coolfluid_mesh_gmsh_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_gmsh )
