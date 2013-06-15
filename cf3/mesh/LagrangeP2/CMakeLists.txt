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
  Quad3D.hpp
  Quad3D.cpp
  Triag.hpp
  Triag.cpp
  Triag2D.hpp
  Triag2D.cpp
)

coolfluid3_add_library( TARGET  coolfluid_mesh_lagrangep2 
                        KERNEL
                        SOURCES ${coolfluid_mesh_lagrangep2_files}
                        LIBS    coolfluid_mesh_lagrangep1 )
