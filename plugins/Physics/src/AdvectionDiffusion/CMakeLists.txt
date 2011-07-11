# TQ: NOTE
#     I leave the sources here but deactivate this code because
#     it follows the old Physics API.
#     The owner of this module should upgrade it to the new API or remove it.

list( APPEND coolfluid_advectiondiffusion_condition FALSE )

list( APPEND coolfluid_advectiondiffusion_files
  Physics.hpp
  LibAdvectionDiffusion.cpp
  LibAdvectionDiffusion.hpp
  State1D.cpp
  State1D.hpp
  State2D.cpp
  State2D.hpp
)

list( APPEND coolfluid_advectiondiffusion_cflibs coolfluid_solver )

coolfluid_add_library( coolfluid_advectiondiffusion )

