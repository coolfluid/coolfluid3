list( APPEND coolfluid_advectiondiffusion_files
  Physics.hpp
  LibAdvectionDiffusion.cpp
  LibAdvectionDiffusion.hpp
  State.cpp
  State.hpp
)

list( APPEND coolfluid_advectiondiffusion_cflibs coolfluid_solver )

coolfluid_add_library( coolfluid_advectiondiffusion )

