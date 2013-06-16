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

list( APPEND coolfluid_math_lss_trilinos_files
    Trilinos/BelosGMRESParameters.hpp
    Trilinos/BelosGMRESParameters.cpp
    Trilinos/DirectStrategy.hpp
    Trilinos/DirectStrategy.cpp
    Trilinos/ParameterList.hpp
    Trilinos/ParameterList.cpp
    Trilinos/ParameterListDefaults.hpp
    Trilinos/RCGStrategy.hpp
    Trilinos/RCGStrategy.cpp
    Trilinos/TekoBlockedOperator.hpp
    Trilinos/TekoBlockedOperator.cpp
    Trilinos/ThyraVector.hpp
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

if(CF3_HAVE_TRILINOS)
    list( APPEND coolfluid_math_lss_files ${coolfluid_math_lss_trilinos_files} )
    list( APPEND coolfluid_math_lss_defs  CF3_HAVE_TRILINOS )
else()
    coolfluid_mark_not_orphan( ${coolfluid_math_lss_trilinos_files} )
endif()

coolfluid3_add_library( TARGET      coolfluid_math_lss
                        KERNEL
                        DEFINITIONS ${coolfluid_math_lss_defs}
                        SOURCES     ${coolfluid_math_lss_files}
                        INCLUDES    ${TRILINOS_INCLUDE_DIRS}
                        LIBS        coolfluid_math coolfluid_common
                                    ${TRILINOS_LIBRARIES} )
