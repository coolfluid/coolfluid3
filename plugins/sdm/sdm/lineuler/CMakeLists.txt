# coolfluid_sdm_lineuler

list( APPEND coolfluid_sdm_lineuler_files
  LibLinEuler.hpp
  LibLinEuler.cpp
  BCWall2D.hpp
  BCWall2D.cpp
  BCWallNonUniformMeanflow2D.hpp
  BCWallNonUniformMeanflow2D.cpp
  ConvectionNonUniformMeanflow2D.hpp
  ConvectionNonUniformMeanflow2D.cpp
  Convection2D.hpp
  Convection2D.cpp
  Convection3D.hpp
  Convection3D.cpp
  FixMeanFlowBoundaryLayer.hpp
  FixMeanFlowBoundaryLayer.cpp
  BCSubsonicOutletThompson2D.hpp
  BCSubsonicOutletThompson2D.cpp
  BCSubsonicOutlet2D.hpp
  BCSubsonicOutlet2D.cpp
  InitAcousticPulse.hpp
  InitAcousticPulse.cpp
  InitAcousticVorticityPulse.hpp
  InitAcousticVorticityPulse.cpp
  ComputeCharacteristicVariables.hpp
  ComputeCharacteristicVariables.cpp
  SourceMeanflow2D.hpp
  SourceMeanflow2D.cpp
  SourceMonopole2D.hpp
  SourceMonopole2D.cpp
  SourceMonopole3D.hpp
  SourceMonopole3D.cpp
  SourceLighthill2D.hpp
  SourceLighthill2D.cpp
)

list( APPEND coolfluid_sdm_lineuler_cflibs coolfluid_sdm coolfluid_physics_lineuler )

coolfluid_add_library( coolfluid_sdm_lineuler )

