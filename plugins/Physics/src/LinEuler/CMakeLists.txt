# coolfluid_lineuler

list( APPEND coolfluid_physics_lineuler_files
  LibLinEuler.cpp
  LibLinEuler.hpp
)

list( APPEND coolfluid_lineuler_cflibs coolfluid_physics )

coolfluid_add_library( coolfluid_physics_lineuler )

