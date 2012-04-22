# coolfluid_physics_lineuler

list( APPEND coolfluid_physics_lineuler_files
  Cons2D.cpp
  Cons2D.hpp
  Cons3D.cpp
  Cons3D.hpp
  LibLinEuler.cpp
  LibLinEuler.hpp
  LinEuler2D.cpp
  LinEuler2D.hpp
  LinEuler3D.cpp
  LinEuler3D.hpp
)

list( APPEND coolfluid_physics_lineuler_cflibs coolfluid_physics )

coolfluid_add_library( coolfluid_physics_lineuler )

