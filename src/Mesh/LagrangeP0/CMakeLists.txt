list( APPEND coolfluid_mesh_lagrangep0_files
  ElementTypes.hpp
  LibLagrangeP0.hpp
  LibLagrangeP0.cpp
  Point.hpp
  Point.cpp
  Point1D.hpp
  Point1D.cpp
  Point2D.hpp
  Point2D.cpp
  Point3D.hpp
  Point3D.cpp
  Triag.hpp
  Triag.cpp
)

list( APPEND coolfluid_mesh_lagrangep0_cflibs coolfluid_mesh )

set( coolfluid_mesh_lagrangep0_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_lagrangep0 )
