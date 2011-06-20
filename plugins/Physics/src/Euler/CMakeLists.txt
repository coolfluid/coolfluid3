list( APPEND coolfluid_euler_files
  Cons1D.hpp
  Cons1D.cpp
  LibEuler.cpp
  LibEuler.hpp
  Physics.hpp
  Roe1D.hpp
  Roe1D.cpp
)

list( APPEND coolfluid_euler_cflibs coolfluid_solver )
coolfluid_add_library( coolfluid_euler )

