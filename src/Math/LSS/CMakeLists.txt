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
list(APPEND coolfluid_lss_includedirs ${TRILINOS_INCLUDE_DIR})
    list( APPEND coolfluid_lss_libs ${TRILINOS_LIBRARIES} )
    ADD_DEFINITIONS(-DCF_HAVE_TRILINOS)
else()
  coolfluid_mark_not_orphan(
  Trilinos/TrilinosMatrix.hpp
  Trilinos/TrilinosMatrix.cpp
  Trilinos/TrilinosVector.hpp
  Trilinos/TrilinosVector.cpp
  )
endif()

set( coolfluid_lss_kernellib TRUE )

coolfluid_add_library( coolfluid_lss )
