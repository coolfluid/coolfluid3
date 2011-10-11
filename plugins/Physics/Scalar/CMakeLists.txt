# coolfluid_physics_scalar

list( APPEND coolfluid_physics_scalar_files
  LibScalar.hpp
  LibScalar.cpp
# scalar 1D
  Scalar1D.hpp
  Scalar1D.cpp
  LinearAdv1D.hpp
  LinearAdv1D.cpp
# scalar 2D
  Scalar2D.hpp
  Scalar2D.cpp
  LinearAdv2D.hpp
  LinearAdv2D.cpp
  RotationAdv2D.hpp
  RotationAdv2D.cpp
  Burgers2D.hpp
  Burgers2D.cpp
  Diffusion2D.hpp
  Diffusion2D.cpp
# scalar 3D
  LinearAdv3D.hpp
  LinearAdv3D.cpp
  Scalar3D.hpp
  Scalar3D.cpp
# scalar system 2D
  ScalarSys2D.hpp
  ScalarSys2D.cpp
  LinearAdvSys2D.hpp
  LinearAdvSys2D.cpp
)

list( APPEND coolfluid_physics_scalar_cflibs coolfluid_physics )

coolfluid_add_library( coolfluid_physics_scalar )

