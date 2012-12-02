list( APPEND coolfluid_physics_euler_files
  LibEuler.cpp
  LibEuler.hpp
  
  euler1d/Types.hpp
  euler1d/Data.hpp
  euler1d/Data.cpp
  euler1d/Functions.hpp
  euler1d/Functions.cpp
  
  euler2d/Types.hpp
  euler2d/Data.hpp
  euler2d/Data.cpp
  euler2d/Functions.hpp
  euler2d/Functions.cpp
)

list( APPEND coolfluid_physics_euler_cflibs coolfluid_physics )

coolfluid_add_library( coolfluid_physics_euler )
