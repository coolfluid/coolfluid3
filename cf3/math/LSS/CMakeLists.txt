coolfluid_find_orphan_files()

list( APPEND coolfluid_math_lss_files
  LibLSS.hpp
  LibLSS.cpp
  System.cpp
  System.hpp
  Matrix.hpp
  Vector.hpp
  BlockAccumulator.hpp
  EmptyLSS/EmptyLSSVector.hpp
  EmptyLSS/EmptyLSSMatrix.hpp
)

list( APPEND coolfluid_math_lss_libs coolfluid_common)

# happen to have trilinos
if(CF3_HAVE_TRILINOS)
  list( APPEND coolfluid_math_lss_files
    Trilinos/TrilinosMatrix.hpp
    Trilinos/TrilinosMatrix.cpp
    Trilinos/TrilinosVector.hpp
    Trilinos/TrilinosVector.cpp
    )
    list( APPEND coolfluid_math_lss_includedirs ${TRILINOS_INCLUDE_DIRS})
    list( APPEND coolfluid_math_lss_libs ${TRILINOS_LIBRARIES} )
    ADD_DEFINITIONS(-DCF3_HAVE_TRILINOS)
else()
  coolfluid_mark_not_orphan(
  Trilinos/TrilinosMatrix.hpp
  Trilinos/TrilinosMatrix.cpp
  Trilinos/TrilinosVector.hpp
  Trilinos/TrilinosVector.cpp
  )
endif()

set( coolfluid_math_lss_kernellib TRUE )

coolfluid_add_library( coolfluid_math_lss )
