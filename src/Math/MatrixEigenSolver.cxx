#include "JacobiEigenSolver.hh"
#include "Common/NotImplementedException.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace MathTools {

//////////////////////////////////////////////////////////////////////////////

MatrixEigenSolver*
MatrixEigenSolver::create(const CFuint& size, const bool& isSymmetric)
{
  if ( isSymmetric )
  {
    if ( size > 10 )
      std::cout << "MatrixEigenSolver::create() : size of matrix is greater then 10. a better algorithm should be used" << std::endl;

    return new JacobiEigenSolver();
  }
  throw Common::NotImplementedException (FromHere(),"MatrixEigenSolver::create() cannot invert non symmetric matrices");
}

//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////
