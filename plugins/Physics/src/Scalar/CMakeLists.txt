# coolfluid_physics_scalar

list( APPEND coolfluid_physics_scalar_files
  LibScalar.cpp
  LibScalar.hpp
  Scalar2D.hpp
  Scalar2D.cpp
  Scalar3D.hpp
  Scalar3D.cpp
)

list( APPEND coolfluid_physics_scalar_cflibs coolfluid_common )

coolfluid_add_library( coolfluid_physics_scalar )

