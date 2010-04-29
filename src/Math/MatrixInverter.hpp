#ifndef CF_Math_MatrixInverter_hpp
#define CF_Math_MatrixInverter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/RealMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// This class represents a matrix inverter
/// @author Andrea Lani
/// @author Tiago Quintino
class Math_API MatrixInverter {
public:

  /// Default constructor without arguments
  MatrixInverter() {}

  /// Default destructor
  virtual ~MatrixInverter() {}

  /// Factory method to create an inverter suitable for the given size
  static MatrixInverter* create (const Uint& size, const bool& isDiagonal);

  /// Factory method to create an inverter suitable for non square matrices (pseudo-inverse)
  static MatrixInverter* create (const Uint& nbRows, const Uint& nbCols, const bool& isDiagonal);

  /// Invert the given matrix a and put the result in x
  /// @param a  matrix to invert
  /// @param x  result of the matrix inversion
  virtual void invert(const RealMatrix& a, RealMatrix& x) = 0;

}; // end of class MatrixInverter

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_MatrixInverter_hpp
