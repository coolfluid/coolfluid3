coolfluid_find_orphan_files()

list( APPEND coolfluid_math_lss_files
  LibLSS.hpp
  LibLSS.cpp
  System.cpp
  System.hpp
  Matrix.hpp
  Vector.hpp
  BlockAccumulator.hpp
  SolutionStrategy.hpp
  SolveLSS.hpp
  SolveLSS.cpp
  ZeroLSS.hpp
  ZeroLSS.cpp
  EmptyLSS/EmptyLSSVector.hpp
  EmptyLSS/EmptyLSSVector.cpp
  EmptyLSS/EmptyLSSMatrix.hpp
  EmptyLSS/EmptyLSSMatrix.cpp
  EmptyLSS/EmptyStrategy.hpp
  EmptyLSS/EmptyStrategy.cpp
)

list( APPEND coolfluid_math_lss_libs coolfluid_math coolfluid_common )

list( APPEND coolfluid_math_lss_trilinos_files
    Trilinos/BelosGMRESParameters.hpp
    Trilinos/BelosGMRESParameters.cpp
    Trilinos/ParameterList.hpp
    Trilinos/ParameterList.cpp
    Trilinos/ParameterListDefaults.hpp
    Trilinos/ThyraMultiVector.hpp
    Trilinos/ThyraOperator.hpp
    Trilinos/TrilinosCrsMatrix.hpp
    Trilinos/TrilinosCrsMatrix.cpp
    Trilinos/TrilinosDetail.hpp
    Trilinos/TrilinosDetail.cpp
    Trilinos/TrilinosFEVbrMatrix.hpp
    Trilinos/TrilinosFEVbrMatrix.cpp
    Trilinos/TrilinosStratimikosStrategy.hpp
    Trilinos/TrilinosStratimikosStrategy.cpp
    Trilinos/TrilinosVector.hpp
    Trilinos/TrilinosVector.cpp
)

# happen to have trilinos
if(CF3_HAVE_TRILINOS)
    list( APPEND coolfluid_math_lss_files ${coolfluid_math_lss_trilinos_files} )
    list( APPEND coolfluid_math_lss_includedirs ${TRILINOS_INCLUDE_DIRS})
    list( APPEND coolfluid_math_lss_libs ${TRILINOS_LIBRARIES} )
    ADD_DEFINITIONS(-DCF3_HAVE_TRILINOS)
else()
  coolfluid_mark_not_orphan( ${coolfluid_math_lss_trilinos_files} )
endif()

set( coolfluid_math_lss_kernellib TRUE )

coolfluid_add_library( coolfluid_math_lss )
