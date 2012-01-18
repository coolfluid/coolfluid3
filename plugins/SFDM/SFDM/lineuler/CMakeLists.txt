# coolfluid_sfdm_lineuler

list( APPEND coolfluid_sfdm_lineuler_files
  LibLinEuler.hpp
  LibLinEuler.cpp
  Convection2D.hpp
  Convection2D.cpp
  InitAcousticVorticityPulse.hpp
  InitAcousticVorticityPulse.cpp
)

list( APPEND coolfluid_sfdm_lineuler_cflibs coolfluid_sfdm coolfluid_physics_lineuler )

coolfluid_add_library( coolfluid_sfdm_lineuler )

