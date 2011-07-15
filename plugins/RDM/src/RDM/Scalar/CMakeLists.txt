########################################################################
# coolfluid_rdm_scalar

list( APPEND coolfluid_rdm_scalar_files
  LibScalar.cpp
  LibScalar.hpp
ScalarLinearLDA.cpp
ScalarLinearSUPG.cpp
ScalarLinearLF.cpp
ScalarLinearN.cpp
ScalarLinearB.cpp
ScalarNonLinearLDA.cpp
ScalarNonLinearSUPG.cpp
ScalarNonLinearLF.cpp
ScalarNonLinearN.cpp
ScalarNonLinearB.cpp
)

list( APPEND coolfluid_rdm_scalar_cflibs coolfluid_physics_scalar coolfluid_rdm_schemes )

coolfluid_add_library( coolfluid_rdm_scalar )
