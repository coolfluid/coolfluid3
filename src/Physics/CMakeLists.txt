coolfluid_find_orphan_files()

list( APPEND coolfluid_physics_files
  LibPhysics.hpp
  LibPhysics.cpp
  PhysModel.cpp
  PhysModel.hpp
  VariableManager.hpp
  VariableManager.cpp
  Variables.hpp
  Variables.cpp
  # a concrete physical model where the physics are dynamically configured
  DynamicModel.cpp
  DynamicModel.hpp
  DynamicVars.cpp
  DynamicVars.hpp
)

set( coolfluid_physics_kernellib TRUE )

list(APPEND coolfluid_physics_cflibs coolfluid_math )

coolfluid_add_library( coolfluid_physics )
