# coolfluid_sdm_lineuler

list( APPEND coolfluid_sdm_lineuler_files
  LibLinEuler.hpp
  LibLinEuler.cpp
  Convection2D.hpp
  Convection2D.cpp
#  BCChar2D.hpp
#  BCChar2D.cpp
#  BCNonReflective2D.hpp
#  BCNonReflective2D.cpp
#  NonReflectiveConvection2D.hpp
#  NonReflectiveConvection2D.cpp
  InitAcousticVorticityPulse.hpp
  InitAcousticVorticityPulse.cpp
  ComputeCharacteristicVariables.hpp
  ComputeCharacteristicVariables.cpp
)

list( APPEND coolfluid_sdm_lineuler_cflibs coolfluid_sdm coolfluid_physics_lineuler )

coolfluid_add_library( coolfluid_sdm_lineuler )

