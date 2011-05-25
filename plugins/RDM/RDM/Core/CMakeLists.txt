list( APPEND coolfluid_rdm_core_files
  LibCore.cpp
  LibCore.hpp
  RKRD.cpp
  RKRD.hpp
  SupportedTypes.hpp
# wizards
  SteadyExplicit.hpp
  SteadyExplicit.cpp
  MySim.hpp
  MySim.cpp
# actions
  ElementLoop.hpp
  BoundaryTerm.hpp
  BoundaryTerm.cpp
  DomainTerm.hpp
  DomainTerm.cpp
  BcDirichlet.hpp
  BcDirichlet.cpp
  BcBase.hpp
  SchemeBase.hpp
  WeakDirichlet.cpp
  WeakDirichlet.hpp
  Cleanup.hpp
  Cleanup.cpp
# time stepping
  ForwardEuler.hpp
  ForwardEuler.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp
# physical models
  Euler2D.hpp
  Euler2D.cpp
  LinearAdv2D.hpp
  LinearAdv2D.cpp
  LinearAdvSys2D.hpp
  LinearAdvSys2D.cpp
  RotationAdv2D.hpp
  RotationAdv2D.cpp
  Burgers2D.hpp
  Burgers2D.cpp
)

list( APPEND coolfluid_rdm_core_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_mesh_actions coolfluid_solver_actions coolfluid_solver )

coolfluid_add_library( coolfluid_rdm_core )
