#ifndef CF_Math_IntersectSolver_hh
#define CF_Math_IntersectSolver_hh

////////////////////////////////////////////////////////////////////////////////

#include "MatrixIntersect.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// This class represents a eigen value solver for symmetric matrices
/// based on Jacobi algorithm
/// @author Jurek Majewski
class Math_API IntersectSolver : public MatrixIntersect
{
public:

  /// Default constructor without arguments
  IntersectSolver() : MatrixIntersect() {}

  /// Default destructor
  ~IntersectSolver()  {}

  /// Finds an eigenvalues and eigenvectors of matrix a
  /// @param a    input matrix - it will be modified during calculation
  /// @param r    result: left eigenvectors matrix
  /// @param lam  result: vector of eigenvalues
  void intersectCalc( const RealMatrix& a, const RealMatrix& b, RealMatrix& res );

}; // end of class IntersectSolver

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_IntersectSolver_hh
