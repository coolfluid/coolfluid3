#ifndef CF_Math_Inverter4_hh
#define CF_Math_Inverter4_hh

////////////////////////////////////////////////////////////////////////////////

#include "Math/MatrixInverter.hpp"
#include "Math/MatrixInverterT.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// Matrix inverter but using a abstract interface
/// @author Andrea Lani
/// @author Tiago Quintino
template < unsigned int SIZE >
struct InverterT : public MatrixInverter
{
  /// Invert the given matrix a and put the result in x
  /// @param a  matrix to invert
  /// @param x  result of the matrix inversion
  virtual void invert(const RealMatrix& a, RealMatrix& x)  {  m_inverter.invert(a,x);  }

private:

  /// the actual inverter object
  MatrixInverterT<SIZE> m_inverter;

}; // end of class Inverter4

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_Inverter4_hh
