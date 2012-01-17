# coolfluid_sfdm_scalar

list( APPEND coolfluid_sfdm_scalar_files
  LibScalar.hpp
  LibScalar.cpp
# scalar 1D
  LinearAdvection1D.hpp
  LinearAdvection1D.cpp
# scalar 2D
  LinearAdvection2D.hpp
  LinearAdvection2D.cpp
  RotationAdvection2D.hpp
  RotationAdvection2D.cpp
)

list( APPEND coolfluid_sfdm_scalar_cflibs coolfluid_sfdm )

coolfluid_add_library( coolfluid_sfdm_scalar )

