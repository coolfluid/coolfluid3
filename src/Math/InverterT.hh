#ifndef COOLFluiD_MathTools_Inverter4_hh
#define COOLFluiD_MathTools_Inverter4_hh

//////////////////////////////////////////////////////////////////////////////

#include "MathTools/MatrixInverter.hh"
#include "MathTools/MatrixInverterT.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace MathTools {

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_MathTools_Inverter4_hh
