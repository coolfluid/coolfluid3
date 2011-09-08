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
if(CF_HAVE_TRILINOS)
list( APPEND coolfluid_lss_files
  Trilinos/TrilinosMatrix.hpp
  Trilinos/TrilinosMatrix.cpp
  Trilinos/TrilinosVector.hpp
  Trilinos/TrilinosVector.cpp
)
list( APPEND coolfluid_lss_libs ${TRILINOS_LIBRARIES} )
ADD_DEFINITIONS(-DCF_HAVE_TRILINOS)
endif()

set( coolfluid_lss_kernellib TRUE )

coolfluid_add_library( coolfluid_lss )








