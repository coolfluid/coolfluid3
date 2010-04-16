CF_FIND_ORPHAN_FILES()

LIST ( APPEND Math_files
FindMinimum.hpp
MatrixInverter.cpp
Cmp.hpp
MatrixEigenSolver.hpp
IntersectSolver.hpp
JacobiEigenSolver.cpp
LExprOp.hpp
LinearFunctor.hpp
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
ConstantFunctor.hpp
InverterDiag.hpp
LUInverter.cpp
NExprOp.hpp
BinaryOp.hpp
CFSliceMatrix.hpp
MathChecks.hpp
Math.hpp
MathExceptions.hpp
MathExceptions.cpp
FunctionParser.hpp
FunctionParser.cpp
CFMatrix.hpp
CFSliceVector.hpp
MatrixIntersect.cpp
ExprOp.hpp
MatrixEigenSolver.cpp
JacobiEigenSolver.hpp
IntersectSolver.cpp
CFVector.hpp
LUInverterT.hpp
LUInverterT.ci
MatrixInverterT.hpp
SVDInverter.hpp
SVDInverter.cpp
RCM.hpp
RCM.cpp
VectorialFunction.hpp
VectorialFunction.cpp
)

LIST ( APPEND Math_cflibs Common )

SET  ( Math_kernellib TRUE )

CF_ADD_LIBRARY ( Math )

