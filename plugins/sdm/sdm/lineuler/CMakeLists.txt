# coolfluid_sdm_lineuler

list( APPEND coolfluid_sdm_lineuler_files
  LibLinEuler.hpp
  LibLinEuler.cpp
  Convection2D.hpp
  Convection2D.cpp
  InitAcousticVorticityPulse.hpp
  InitAcousticVorticityPulse.cpp
)

list( APPEND coolfluid_sdm_lineuler_cflibs coolfluid_sdm coolfluid_physics_lineuler )

coolfluid_add_library( coolfluid_sdm_lineuler )

