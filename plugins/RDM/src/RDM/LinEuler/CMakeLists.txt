########################################################################
# coolfluid_rdm_lineuler

list( APPEND coolfluid_rdm_lineuler_files
  # library
  LibLinEuler.cpp
  LibLinEuler.hpp
  # domain terms (schemes)
  LinEulerLDA.cpp
)

list( APPEND coolfluid_rdm_lineuler_cflibs  coolfluid_physics_lineuler coolfluid_rdm_schemes )

coolfluid_add_library( coolfluid_rdm_lineuler )
