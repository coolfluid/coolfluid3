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

coolfluid3_add_library( TARGET  coolfluid_mesh_lagrangep2b
                        KERNEL
                        SOURCES ${coolfluid_mesh_lagrangep2b_files}
                        LIBS    coolfluid_mesh_lagrangep2 )
