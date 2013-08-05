coolfluid_find_orphan_files()

list( APPEND coolfluid_physics_files
  LibPhysics.hpp
  LibPhysics.cpp
  MatrixTypes.hpp
  Consts.hpp
  PhysModel.cpp
  PhysModel.hpp
  Variables.hpp
  Variables.cpp
  # a concrete physical model where the physics are dynamically configured
  DynamicModel.cpp
  DynamicModel.hpp
  DynamicVars.cpp
  DynamicVars.hpp
)

coolfluid3_add_library( TARGET    coolfluid_physics
                        KERNEL
                        SOURCES   ${coolfluid_physics_files}
                        LIBS      coolfluid_math )
