list( APPEND coolfluid_mesh_lagrangep1_files
  LibLagrangeP1.hpp
  LibLagrangeP1.cpp
  Triag.hpp
  Triag.cpp
  Triag2D.hpp
  Triag2D.cpp
)

list( APPEND coolfluid_mesh_lagrangep1_cflibs coolfluid_mesh )

set( coolfluid_mesh_lagrangep1_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_lagrangep1 )
