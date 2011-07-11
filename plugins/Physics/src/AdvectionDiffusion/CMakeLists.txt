list( APPEND coolfluid_advectiondiffusion_files
  Physics.hpp
  LibAdvectionDiffusion.cpp
  LibAdvectionDiffusion.hpp
  State1D.cpp
  State1D.hpp
  State2D.cpp
  State2D.hpp
)

list( APPEND coolfluid_advectiondiffusion_condition FALSE )

list( APPEND coolfluid_advectiondiffusion_cflibs coolfluid_solver )

coolfluid_add_library( coolfluid_advectiondiffusion )

