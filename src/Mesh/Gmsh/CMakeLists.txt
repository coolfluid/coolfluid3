list( APPEND coolfluid_gmsh_files
  CWriter.hpp
  CWriter.cpp
  CReader.hpp
  CReader.cpp
  LibGmsh.cpp
  LibGmsh.hpp
  Shared.cpp
  Shared.hpp
)

list( APPEND coolfluid_gmsh_cflibs coolfluid_mesh )

set( coolfluid_gmsh_kernellib TRUE )

coolfluid_add_library( coolfluid_gmsh )
