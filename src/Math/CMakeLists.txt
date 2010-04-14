CF_FIND_ORPHAN_FILES()

# try to compile using boost
SET ( CMAKE_REQUIRED_INCLUDES ${Boost_INCLUDE_DIR} )
CHECK_CXX_SOURCE_COMPILES (
	"#include <boost/math/special_functions/erf.hpp>
	int main(int argc, char* argv[])
	{
	 boost::math::erfc(0.);
	}"
	CF_HAVE_BOOST_ERFC )

LIST ( APPEND MathTools_files
FindMinimum.hh
MatrixInverter.cxx
Cmp.hh
MatrixEigenSolver.hh
IntersectSolver.hh
JacobiEigenSolver.cxx
LExprOp.hh
LinearFunctor.hh
MathConsts.hh
MathFunctions.hh
ZeroDeterminantException.hh
ZeroDeterminantException.cxx
RealMatrix.hh
RealMatrix.cxx
RealVector.cxx
MatrixIntersect.hh
MatrixInverter.hh
InverterT.hh
OutOfBoundsException.hh
LUInverter.hh
RealVector.hh
ConstantFunctor.hh
InverterDiag.hh
LUInverter.cxx
NExprOp.hh
BinaryOp.hh
CFSliceMatrix.hh
MathChecks.hh
MacroLET.hh
MathTools.hh
FunctionParser.hh
FunctionParser.cxx
CFMatrix.hh
CFSliceVector.hh
MatrixIntersect.cxx
ExprOp.hh
MatrixEigenSolver.cxx
JacobiEigenSolver.hh
VectorLET.ci
VectorLET.hh
IntersectSolver.cxx
CFVector.hh
MatrixLET.ci
MatrixLET.hh
LUInverterT.hh
LUInverterT.ci
MatrixInverterT.hh
SVDInverter.hh
SVDInverter.cxx
RCM.h
RCM.cxx
)

LIST ( APPEND MathTools_cflibs Common )

CF_ADD_KERNEL_LIBRARY ( MathTools )


