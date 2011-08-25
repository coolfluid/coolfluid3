coolfluid_find_orphan_files()

list( APPEND coolfluid_lssinterface_files
  System.hpp
  System.cpp
  Matrix.hpp
  Vector.hpp
  BlockAccumulator.hpp
)

list( APPEND coolfluid_lssinterface_libs coolfluid_common_libs ${CF_BOOST_LIBRARIES})

set( coolfluid_lssinterface_kernellib TRUE )

coolfluid_add_library( coolfluid_lssinterface )

# happen to have trilinos
if(CF_HAVE_TRILINOS)
add_subdirectory( Trilinos )
endif()





