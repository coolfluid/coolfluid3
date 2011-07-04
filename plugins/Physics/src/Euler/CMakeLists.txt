list( APPEND coolfluid_euler_files
  Cons1D.hpp
  Cons1D.cpp
  Cons2D.hpp
  Cons2D.cpp
  LibEuler.cpp
  LibEuler.hpp
  Physics.hpp
  Roe1D.hpp
  Roe1D.cpp
  Roe2D.hpp
  Roe2D.cpp
)

list( APPEND coolfluid_euler_cflibs coolfluid_solver )
coolfluid_add_library( coolfluid_euler )

