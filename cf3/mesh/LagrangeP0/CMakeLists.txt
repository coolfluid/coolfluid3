list( APPEND coolfluid_mesh_lagrangep0_files
  API.hpp
  ElementTypes.hpp
  Hexa.hpp
  Hexa.cpp
  LibLagrangeP0.hpp
  LibLagrangeP0.cpp
  Line.hpp
  Line.cpp
  Point.hpp
  Point.cpp
  Point1D.hpp
  Point1D.cpp
  Point2D.hpp
  Point2D.cpp
  Point3D.hpp
  Point3D.cpp
  Prism.hpp
  Prism.cpp
  Quad.hpp
  Quad.cpp
  Tetra.hpp
  Tetra.cpp
  Triag.hpp
  Triag.cpp
)

coolfluid3_add_library( TARGET  coolfluid_mesh_lagrangep0 
                        KERNEL
                        SOURCES ${coolfluid_mesh_lagrangep0_files}
                        LIBS    coolfluid_mesh )
