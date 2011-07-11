# TQ: NOTE
#     I leave the sources here but deactivate this code because
#     it follows the old Physics API.
#     The owner of this module should upgrade it to the new API or remove it.

list( APPEND coolfluid_euler_condition FALSE )


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

