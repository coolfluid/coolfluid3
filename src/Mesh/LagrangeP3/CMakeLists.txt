list( APPEND coolfluid_mesh_lagrangep3_files
  ElementTypes.hpp
  LibLagrangeP3.hpp
  LibLagrangeP3.cpp
  Line.hpp
  Line.cpp
  Line2D.hpp
  Line2D.cpp
  Quad.hpp
  Quad.cpp
  Quad2D.hpp
  Quad2D.cpp
  Triag.hpp
  Triag.cpp
  Triag2D.hpp
  Triag2D.cpp
)

list( APPEND coolfluid_mesh_lagrangep3_cflibs coolfluid_mesh_lagrangep0 )

set( coolfluid_mesh_lagrangep3_kernellib TRUE )

coolfluid_add_library( coolfluid_mesh_lagrangep3 )
