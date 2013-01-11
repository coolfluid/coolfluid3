list( APPEND coolfluid_mesh_lagrangep3_files
  API.hpp
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


coolfluid3_add_library( TARGET  coolfluid_mesh_lagrangep3
                        KERNEL
                        SOURCES ${coolfluid_mesh_lagrangep3_files}
                        LIBS    coolfluid_mesh )
