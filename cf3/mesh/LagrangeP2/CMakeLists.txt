list( APPEND coolfluid_mesh_lagrangep2_files
  API.hpp
  ElementTypes.hpp
  LibLagrangeP2.hpp
  LibLagrangeP2.cpp
  Line.hpp
  Line.cpp
  Line1D.hpp
  Line1D.cpp
  Line2D.hpp
  Line2D.cpp
  Point.hpp
  Point.cpp
  Quad.hpp
  Quad.cpp
  Quad2D.hpp
  Quad2D.cpp
  Triag.hpp
  Triag.cpp
  Triag2D.hpp
  Triag2D.cpp
)

list( APPEND coolfluid_mesh_lagrangep2_cflibs coolfluid_mesh_lagrangep1 )

set( coolfluid_mesh_lagrangep2_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_lagrangep2 )
