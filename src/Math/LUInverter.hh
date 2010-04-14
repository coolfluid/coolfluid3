#ifndef COOLFluiD_MathTools_LUInverter_hh
#define COOLFluiD_MathTools_LUInverter_hh

//////////////////////////////////////////////////////////////////////////////

#include <valarray>

#include "RealVector.hh"
#include "MatrixInverter.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace MathTools {

//////////////////////////////////////////////////////////////////////////////

  /// This class represents a solver that uses the LU decomposition
  /// @author Andrea Lani
  /// @author Tiago Quintino
class MathTools_API LUInverter : public MatrixInverter {
public:

  /// Default constructor without arguments
  LUInverter(const CFuint& n);

  /// Default destructor
  ~LUInverter();

  /// Invert the given matrix a and put the result in x
  void invert(const RealMatrix& a, RealMatrix& x);

private: //helper functions

  /// Factorize the matrix
  void factLU();

  /// Solve with forward and backward substitution
  void solveForwBack();

private: //data

  CFuint                _n;

  std::valarray<CFuint> _indx;

  RealVector             _tempcol;

  RealVector             _vv;

  RealMatrix             _a;

}; // end of class LUInverter

//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_MathTools_LUInverter_hh
