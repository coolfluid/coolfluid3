#ifndef CF_Math_InverterDiag_hh
#define CF_Math_InverterDiag_hh

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixInverter.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// Matrix inverter for diagonal matrices but using a abstract interface
/// @author Andrea Lani
/// @author Tiago Quintino
struct Math_API InverterDiag : public MatrixInverter {

  /// Invert the given matrix a and put the result in x
  /// @param a  matrix to invert
  /// @param x  result of the matrix inversion
  virtual void invert(const RealMatrix& a, RealMatrix& x)  {  a.invertDiag(x);  }

}; // end of class InverterDiag

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_InverterDiag_hh
