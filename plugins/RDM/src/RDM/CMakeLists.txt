list( APPEND coolfluid_rdm_files
  LibRDM.cpp
  LibRDM.hpp
  RKRD.cpp
  RKRD.hpp
  SupportedTypes.hpp
# wizards
  ScalarAdvection.hpp
  ScalarAdvection.cpp
# actions
  Action.hpp
  Action.cpp
  BoundaryTerm.hpp
  BoundaryTerm.cpp
  DomainTerm.hpp
  DomainTerm.cpp
  BcDirichlet.hpp
  BcDirichlet.cpp
  Cleanup.hpp
  Cleanup.cpp
  UpdateSolution.hpp
  UpdateSolution.cpp
# schemes
  FluxOp2D.hpp
  LDA.hpp
  LDA.cpp
  Blended.hpp
  Blended.cpp
  SchemeLDA.hpp
  SchemeN.hpp
  SchemeB.hpp
# physical models
  LinearAdv2D.hpp
  LinearAdv2D.cpp
  RotationAdv2D.hpp
  RotationAdv2D.cpp
  Burgers2D.hpp
  Burgers2D.cpp
)

list( APPEND coolfluid_rdm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_mesh_actions coolfluid_solver_actions coolfluid_solver )

coolfluid_add_library( coolfluid_rdm )
