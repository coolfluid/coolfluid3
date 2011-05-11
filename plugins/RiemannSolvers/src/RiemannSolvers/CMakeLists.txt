list( APPEND coolfluid_riemannsolvers_files
  LibRiemannSolvers.hpp
  LibRiemannSolvers.cpp
  RiemannSolver.hpp
  RiemannSolver.cpp
  RoeCons1D.hpp
  RoeCons1D.cpp
  RoeCons2D.hpp
  RoeCons2D.cpp
)

list( APPEND coolfluid_riemannsolvers_cflibs coolfluid_math )

coolfluid_add_library( coolfluid_riemannsolvers )
