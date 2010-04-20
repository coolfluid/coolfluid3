CF_FIND_ORPHAN_FILES()

LIST ( APPEND Math_files
BoostHeaders.hpp
FindMinimum.hpp
MatrixInverter.cpp
MatrixEigenSolver.hpp
IntersectSolver.hpp
JacobiEigenSolver.cpp
MathConsts.hpp
MathFunctions.hpp
RealMatrix.hpp
RealMatrix.cpp
RealVector.cpp
MatrixIntersect.hpp
MatrixInverter.hpp
InverterT.hpp
LUInverter.hpp
RealVector.hpp
InverterDiag.hpp
LUInverter.cpp
MatrixSliceT.hpp
MathChecks.hpp
Math.hpp
MathExceptions.hpp
MathExceptions.cpp
FunctionParser.hpp
FunctionParser.cpp
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

LIST ( APPEND Math_cflibs Common )

SET  ( Math_kernellib TRUE )

CF_ADD_LIBRARY ( Math )

ADD_SUBDIRECTORY ( uTests )    # unit tests for Math

