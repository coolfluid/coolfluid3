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
