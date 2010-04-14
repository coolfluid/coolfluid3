#ifndef COOLFluiD_MathTools_InverterDiag_hh
#define COOLFluiD_MathTools_InverterDiag_hh

//////////////////////////////////////////////////////////////////////////////

#include "MathTools/MatrixInverter.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace MathTools {

//////////////////////////////////////////////////////////////////////////////

/// Matrix inverter for diagonal matrices but using a abstract interface
/// @author Andrea Lani
/// @author Tiago Quintino
struct MathTools_API InverterDiag : public MatrixInverter {

  /// Invert the given matrix a and put the result in x
  /// @param a  matrix to invert
  /// @param x  result of the matrix inversion
  virtual void invert(const RealMatrix& a, RealMatrix& x)  {  a.invertDiag(x);  }

}; // end of class InverterDiag

//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_MathTools_InverterDiag_hh
