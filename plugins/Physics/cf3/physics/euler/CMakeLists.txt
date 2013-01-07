list( APPEND coolfluid_physics_euler_files
  LibEuler.cpp
  LibEuler.hpp
  # Euler 1d
  euler1d/Types.hpp
  euler1d/Data.hpp
  euler1d/Data.cpp
  euler1d/Functions.hpp
  euler1d/Functions.cpp
  # Euler 2d
  euler2d/Types.hpp
  euler2d/Data.hpp
  euler2d/Data.cpp
  euler2d/Functions.hpp
  euler2d/Functions.cpp
)

coolfluid3_add_library( TARGET   coolfluid_physics_euler
                        SOURCES  ${coolfluid_physics_euler_files}
                        LIBS     coolfluid_physics )
