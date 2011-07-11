# coolfluid_physics_scalar

list( APPEND coolfluid_physics_scalar_files
  LibScalar.hpp
  LibScalar.cpp
  Scalar2D.hpp
  Scalar2D.cpp
  Scalar3D.hpp
  Scalar3D.cpp
  LinearAdv2D.hpp
  LinearAdv2D.cpp
  RotationAdv2D.hpp
  RotationAdv2D.cpp
  Burgers2D.hpp
  Burgers2D.cpp
)

list( APPEND coolfluid_physics_scalar_cflibs coolfluid_common )

coolfluid_add_library( coolfluid_physics_scalar )

