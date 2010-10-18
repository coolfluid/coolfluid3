list( APPEND coolfluid_gmsh_files
  CWriter.hpp
  CWriter.cpp
  LibGmsh.cpp
  LibGmsh.hpp
)

list( APPEND coolfluid_gmsh_cflibs coolfluid_mesh )

set( coolfluid_gmsh_kernellib TRUE )

coolfluid_add_library( coolfluid_gmsh )
