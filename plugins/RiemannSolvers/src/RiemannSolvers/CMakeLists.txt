# TQ: NOTE
#     I leave the sources here but deactivate this code because
#     it follows the old Physics API.
#     The owner of this module should upgrade it to the new API or remove it.

list( APPEND coolfluid_riemannsolvers_condition FALSE )

list( APPEND coolfluid_riemannsolvers_files
  LibRiemannSolvers.hpp
  LibRiemannSolvers.cpp
  RiemannSolver.hpp
  RiemannSolver.cpp
  Roe.hpp
  Roe.cpp
)

list( APPEND coolfluid_riemannsolvers_cflibs coolfluid_math coolfluid_solver )

coolfluid_add_library( coolfluid_riemannsolvers )
