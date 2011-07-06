list( APPEND coolfluid_physics_navierstokes_files
  LibNavierStokes.cpp
  LibNavierStokes.hpp
  NavierStokes2D.cpp
  NavierStokes2D.hpp
  Cons2D.hpp
  Cons2D.cpp
)

list( APPEND coolfluid_physics_navierstokes_cflibs coolfluid_physics )

coolfluid_add_library( coolfluid_physics_navierstokes )
