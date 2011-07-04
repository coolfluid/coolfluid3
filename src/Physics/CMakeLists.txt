coolfluid_find_orphan_files()

list( APPEND coolfluid_physics_files
  LibPhysics.hpp
  LibPhysics.cpp
  PhysModel.cpp
  PhysModel.hpp
  Variables.hpp
  Variables.cpp
)

set( coolfluid_physics_kernellib TRUE )

list(APPEND coolfluid_physics_cflibs coolfluid_math )

coolfluid_add_library( coolfluid_physics )
