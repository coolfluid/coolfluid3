list( APPEND coolfluid_fvm_files
  LibFVM.cpp
  LibFVM.hpp
  ComputeFlux.hpp
  ComputeFlux.cpp
  FiniteVolume.hpp
  FiniteVolume.cpp
  ForwardEuler.hpp
  ForwardEuler.cpp
  RoeFluxSplitter.hpp
  RoeFluxSplitter.cpp
  ShockTube.hpp
  ShockTube.cpp
)

list( APPEND coolfluid_fvm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_solver_actions coolfluid_solver coolfluid_mesh_actions )

coolfluid_add_library( coolfluid_fvm )
