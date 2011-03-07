list( APPEND coolfluid_rdm_files
  LibRDM.cpp
  LibRDM.hpp
  Action.hpp
  Action.cpp
  BoundaryTerm.hpp
  BoundaryTerm.cpp
  Cleanup.hpp
  Cleanup.cpp
  DomainTerm.hpp
  DomainTerm.cpp
  BcDirichlet.hpp
  BcDirichlet.cpp
  LDA.hpp
  LDA.cpp
  Blended.hpp
  Blended.cpp
  SchemeLDA.hpp
  SchemeN.hpp
  SchemeB.hpp
  RKRD.cpp
  RKRD.hpp
  ScalarAdvection.hpp
  ScalarAdvection.cpp
  SupportedTypes.hpp
  FluxOp2D.hpp
  LinearAdv2D.hpp
  LinearAdv2D.cpp
  RotationAdv2D.hpp
  RotationAdv2D.cpp
  Burgers2D.hpp
  Burgers2D.cpp
)

list( APPEND coolfluid_rdm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_mesh_actions coolfluid_solver_actions coolfluid_solver )

coolfluid_add_library( coolfluid_rdm )
