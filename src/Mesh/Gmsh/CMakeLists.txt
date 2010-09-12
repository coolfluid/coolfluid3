list( APPEND Gmsh_files
  GmshAPI.hpp
  CWriter.hpp
  CWriter.cpp
)

list( APPEND Gmsh_cflibs Mesh )

set( Gmsh_kernellib TRUE )

coolfluid_add_library ( Gmsh )
