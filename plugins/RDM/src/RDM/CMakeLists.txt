list( APPEND coolfluid_rdm_files
  LibRDM.cpp
  LibRDM.hpp
  CSchemeLDAT.hpp
  CSchemeN.hpp
  ResidualDistribution.cpp
  ResidualDistribution.hpp
  RungeKutta.cpp
  RungeKutta.hpp
  ScalarAdvection.hpp
  ScalarAdvection.cpp
  CTakeStep.hpp
  CTakeStep.cpp
  SupportedTypes.hpp
  FluxOp2D.hpp
  LinearAdv2D.hpp
  LinearAdv2D.cpp
  RotationAdv2D.hpp
  RotationAdv2D.cpp
)

list( APPEND coolfluid_rdm_cflibs coolfluid_mesh coolfluid_mesh_sf coolfluid_solver_actions coolfluid_solver )

coolfluid_add_library( coolfluid_rdm )
