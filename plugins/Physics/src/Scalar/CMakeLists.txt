# coolfluid_scalar

list( APPEND coolfluid_scalar_files
  LibScalar.cpp
  LibScalar.hpp
)

list( APPEND coolfluid_scalar_cflibs coolfluid_common )

coolfluid_add_library( coolfluid_scalar )

