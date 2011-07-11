# coolfluid_lineuler

list( APPEND coolfluid_lineuler_files
  LibLinEuler.cpp
  LibLinEuler.hpp
)

list( APPEND coolfluid_lineuler_cflibs coolfluid_common )

coolfluid_add_library( coolfluid_lineuler )

