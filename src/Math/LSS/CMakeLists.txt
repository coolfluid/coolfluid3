coolfluid_find_orphan_files()

list( APPEND coolfluid_lss_files
  System.cpp
  System.hpp
  Matrix.hpp
  Vector.hpp
  BlockAccumulator.hpp
  EmptyLSS/EmptyLSSVector.hpp
  EmptyLSS/EmptyLSSMatrix.hpp
)

list( APPEND coolfluid_lss_libs coolfluid_common ${CF_BOOST_LIBRARIES})

# happen to have trilinos
#if(CF_HAVE_TRILINOS)
#list( APPEND coolfluid_lss_files
#)
#list( APPEND coolfluid_lss_libs coolfluid_common ${CF_BOOST_LIBRARIES}
#)
#endif()

set( coolfluid_lss_kernellib TRUE )

coolfluid_add_library( coolfluid_lss )








