list( APPEND Gmsh_files
  CWriter.hpp
  CWriter.cpp
  LibGmsh.cpp
  LibGmsh.hpp
)

list( APPEND Gmsh_cflibs Mesh )

set( Gmsh_kernellib TRUE )

coolfluid_add_library ( Gmsh )
