list( APPEND coolfluid_rdm_files
  LibRDM.cpp
  LibRDM.hpp
  BcDirichlet.hpp
  BcDirichlet.cpp
  CAction.hpp
  CAction.cpp
  CLDA.hpp
  CLDA.cpp
  CBlended.hpp
  CBlended.cpp
  CSchemeLDAT.hpp
  CSchemeN.hpp
  CSchemeB.hpp
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

list( APPEND coolfluid_rdm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_solver_actions coolfluid_solver )

coolfluid_add_library( coolfluid_rdm )
