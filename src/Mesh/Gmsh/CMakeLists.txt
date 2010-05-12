LIST ( APPEND Gmsh_files
  GmshAPI.hpp
  Writer.hpp
  Writer.cpp
)

LIST ( APPEND Gmsh_cflibs Mesh )

SET  ( Gmsh_kernellib TRUE )

CF_ADD_LIBRARY ( Gmsh )
