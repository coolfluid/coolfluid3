#ifndef CF_Math_JacobiEigenSolver_hpp
#define CF_Math_JacobiEigenSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "MatrixEigenSolver.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// This class represents a eigen value solver for symmetric matrices
/// based on Jacobi algorithm
/// @author Jurek Majewski
class Math_API JacobiEigenSolver : public MatrixEigenSolver
{
public:

  /// Default constructor without arguments
  JacobiEigenSolver() : MatrixEigenSolver() {}

  /// Default destructor
  ~JacobiEigenSolver()  {}

  /// Finds an eigenvalues and eigenvectors of matrix a
  /// @param a    input matrix - it will be modified during calculation
  /// @param r    result: left eigenvectors matrix
  /// @param lam  result: vector of eigenvalues
  void eigenCalc( RealMatrix& a, RealMatrix& r, RealVector& lam );

private:
  void  RotateMatrix( RealMatrix& a,
    Real& g, Real& h, Real& s, Real& c, Real& tau,
    const Uint& i, const Uint& j, const Uint& k, const Uint& l );


}; // end of class JacobiEigenSolver

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_JacobiEigenSolver_hpp
