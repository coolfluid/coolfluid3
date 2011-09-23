list( APPEND coolfluid_physics_navierstokes_files
  LibNavierStokes.cpp
  LibNavierStokes.hpp
  NavierStokes1D.hpp
  NavierStokes1D.cpp
  NavierStokes2D.cpp
  NavierStokes2D.hpp
  NavierStokes3D.cpp
  NavierStokes3D.hpp
  Cons1D.hpp
  Cons1D.cpp
  Cons2D.hpp
  Cons2D.cpp
  Cons3D.hpp
  Cons3D.cpp
  Roe1D.hpp
  Roe1D.cpp
  Roe2D.hpp
  Roe2D.cpp
  Roe3D.hpp
  Roe3D.cpp
)

list( APPEND coolfluid_physics_navierstokes_cflibs coolfluid_physics )

coolfluid_add_library( coolfluid_physics_navierstokes )
