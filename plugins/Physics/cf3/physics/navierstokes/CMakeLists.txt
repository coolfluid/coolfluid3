list( APPEND coolfluid_physics_navierstokes_files
  LibNavierStokes.cpp
  LibNavierStokes.hpp

  # Navier-Stokes 1d
  navierstokes1d/Types.hpp
  navierstokes1d/Data.hpp
  navierstokes1d/Data.cpp
  navierstokes1d/Functions.hpp
  navierstokes1d/Functions.cpp

  # Navier-Stokes 2d
  navierstokes2d/Types.hpp
  navierstokes2d/Data.hpp
  navierstokes2d/Data.cpp
  navierstokes2d/Functions.hpp
  navierstokes2d/Functions.cpp
)

coolfluid3_add_library( TARGET   coolfluid_physics_navierstokes
                        SOURCES  ${coolfluid_physics_navierstokes_files}
                        LIBS     coolfluid_physics_euler )
