#include "JacobiEigenSolver.hpp"
#include "Common/BasicExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

MatrixEigenSolver*
MatrixEigenSolver::create(const Uint& size, const bool& isSymmetric)
{
  if ( isSymmetric )
  {
    if ( size > 10 )
      std::cout << "MatrixEigenSolver::create() : size of matrix is greater then 10. a better algorithm should be used" << std::endl;

    return new JacobiEigenSolver();
  }
  throw Common::NotImplemented (FromHere(),"MatrixEigenSolver::create() cannot invert non symmetric matrices");
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////
