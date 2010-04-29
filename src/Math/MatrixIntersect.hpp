#ifndef CF_Math_MatrixIntersect_hpp
#define CF_Math_MatrixIntersect_hpp

////////////////////////////////////////////////////////////////////////////////

#include "RealMatrix.hpp"
#include "RealVector.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// This class is used to find an intersection of two SPD matrices
/// @author Jurek Majewski
class Math_API MatrixIntersect
{
public:

  /// Default constructor without arguments
  MatrixIntersect()  {}

  /// Default destructor
  virtual ~MatrixIntersect()  {}

  /// Factory method to create an inverter suitable for the
  /// given size
  static MatrixIntersect* create( const Uint& size);

  /// Invert the given matrix a and put the result in x
  /// @param a    first matrix
  /// @param b    second matrix
  /// @param res  result: final matrix
  virtual void intersectCalc( const RealMatrix& a, const RealMatrix& b, RealMatrix& res) = 0;

}; // end of class MatrixIntersect

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_MatrixIntersect_hpp
