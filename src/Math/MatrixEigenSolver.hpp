#ifndef CF_Math_MatrixEigenSolver_hpp
#define CF_Math_MatrixEigenSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "RealMatrix.hpp"
#include "RealVector.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {
    
////////////////////////////////////////////////////////////////////////////////

  /// This class represents a matrix eigen value problem solver
  /// @author Jurek Majewski
class Math_API MatrixEigenSolver
{
public:

  /// Default constructor without arguments
  MatrixEigenSolver()  {}

  /// Default destructor
  virtual ~MatrixEigenSolver()  {}

  /// Factory method to create an inverter suitable for the
  /// given size
  static MatrixEigenSolver* create(const Uint& size,
    const bool& isSymmetric);

  /// Invert the given matrix a and put the result in x
  /// @param a    matrix to invert
  /// @param r    result: left eigenvectors matrix
  /// @param lam  result: vector of eigenvalues
  virtual void eigenCalc( RealMatrix& a, RealMatrix& r, RealVector& lam) = 0;

}; // end of class MatrixEigenSolver

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_MatrixEigenSolver_hpp
