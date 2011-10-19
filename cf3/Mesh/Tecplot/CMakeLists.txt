list( APPEND coolfluid_mesh_tecplot_files
  CWriter.hpp
  CWriter.cpp
  LibTecplot.cpp
  LibTecplot.hpp
  Shared.cpp
  Shared.hpp
)

list( APPEND coolfluid_mesh_tecplot_cflibs coolfluid_mesh )

set( coolfluid_mesh_tecplot_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_tecplot )
