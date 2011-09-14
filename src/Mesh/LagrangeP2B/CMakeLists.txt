list( APPEND coolfluid_mesh_lagrangep2b_files
  API.hpp
  ElementTypes.hpp
  LibLagrangeP2B.hpp
  LibLagrangeP2B.cpp
  Triag.hpp
  Triag.cpp
  Triag2D.hpp
  Triag2D.cpp
)

list( APPEND coolfluid_mesh_lagrangep2b_cflibs coolfluid_mesh_lagrangep2 )

set( coolfluid_mesh_lagrangep2b_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_lagrangep2b )
