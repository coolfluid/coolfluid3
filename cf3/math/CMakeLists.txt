coolfluid_find_orphan_files()

list( APPEND coolfluid_math_files
  LibMath.cpp
  LibMath.hpp
  BoostMath.hpp
  BoundingBox.hpp
  BoundingBox.cpp
  Checks.hpp
  Consts.hpp
  Defs.hpp
  FindMinimum.hpp
  FloatingPoint.hpp
  AnalyticalFunction.hpp
  AnalyticalFunction.cpp
  Functions.hpp
  Hilbert.hpp
  Hilbert.cpp
  Integrate.hpp
  MatrixTypes.hpp
  MatrixTypesConversion.hpp
  MathExceptions.hpp
  MathExceptions.cpp
  VariableManager.hpp
  VariableManager.cpp
  VariablesDescriptor.hpp
  VariablesDescriptor.cpp
  VectorialFunction.hpp
  VectorialFunction.cpp
)

# lssinterface provide connection with linear system solvers
add_subdirectory( LSS )

coolfluid3_add_library( TARGET   coolfluid_math
                        KERNEL
                        SOURCES  ${coolfluid_math_files}
                        LIBS     coolfluid_fparser coolfluid_common  )

