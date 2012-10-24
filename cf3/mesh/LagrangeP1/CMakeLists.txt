list( APPEND coolfluid_mesh_lagrangep1_files
  API.hpp
  ElementTypes.hpp
  Hexa.hpp
  Hexa.cpp
  Hexa3D.hpp
  Hexa3D.cpp
  LibLagrangeP1.hpp
  LibLagrangeP1.cpp
  Line.hpp
  Line.cpp
  Line1D.hpp
  Line1D.cpp
  Line2D.hpp
  Line2D.cpp
  Line3D.hpp
  Line3D.cpp
  Point.hpp
  Point.cpp
  Quad.hpp
  Quad.cpp
  Quad2D.hpp
  Quad2D.cpp
  Quad3D.hpp
  Quad3D.cpp
  Tetra.hpp
  Tetra.cpp
  Tetra3D.hpp
  Tetra3D.cpp
  Triag.hpp
  Triag.cpp
  Triag2D.hpp
  Triag2D.cpp
  Triag3D.hpp
  Triag3D.cpp
)

list( APPEND coolfluid_mesh_lagrangep1_cflibs coolfluid_mesh_lagrangep0 )

set( coolfluid_mesh_lagrangep1_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_lagrangep1 )
