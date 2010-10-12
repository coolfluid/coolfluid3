coolfluid_find_orphan_files()

list( APPEND Math_files
BoostMath.hpp
FindMinimum.hpp
LibMath.cpp
LibMath.hpp
MatrixInverter.cpp
MatrixEigenSolver.hpp
IntersectSolver.hpp
JacobiEigenSolver.cpp
MathConsts.hpp
MathFunctions.hpp
RealMatrix.hpp
MatrixIntersect.hpp
MatrixInverter.hpp
InverterT.hpp
LUInverter.hpp
RealVector.hpp
InverterDiag.hpp
LUInverter.cpp
MatrixSliceT.hpp
MathChecks.hpp
MathExceptions.hpp
MathExceptions.cpp
MatrixT.hpp
VectorSliceT.hpp
MatrixIntersect.cpp
ExprOp.hpp
MatrixEigenSolver.cpp
JacobiEigenSolver.hpp
IntersectSolver.cpp
VectorT.hpp
LUInverterT.hpp
MatrixInverterT.hpp
SVDInverter.hpp
SVDInverter.cpp
VectorialFunction.hpp
VectorialFunction.cpp
)

list( APPEND Math_cflibs fparser Common )

set( Math_kernellib TRUE )

coolfluid_add_library( Math )

add_subdirectory( uTests )    # unit tests for Math

