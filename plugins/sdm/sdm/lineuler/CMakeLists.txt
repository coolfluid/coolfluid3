# coolfluid_sdm_lineuler

list( APPEND coolfluid_sdm_lineuler_files
  LibLinEuler.hpp
  LibLinEuler.cpp
  Convection2D.hpp
  Convection2D.cpp
  BCChar2D.hpp
  BCChar2D.cpp
  BCSubsonicOutlet2D.hpp
  BCSubsonicOutlet2D.cpp
  NonReflectiveConvection2D.hpp
  NonReflectiveConvection2D.cpp
  InitAcousticVorticityPulse.hpp
  InitAcousticVorticityPulse.cpp
  ComputeCharacteristicVariables.hpp
  ComputeCharacteristicVariables.cpp
  SourceMonopole2D.hpp
  SourceMonopole2D.cpp
)

list( APPEND coolfluid_sdm_lineuler_cflibs coolfluid_sdm coolfluid_physics_lineuler )

coolfluid_add_library( coolfluid_sdm_lineuler )

