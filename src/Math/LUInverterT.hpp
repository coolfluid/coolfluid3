#ifndef CF_Math_LUInverterT_hh
#define CF_Math_LUInverterT_hh

////////////////////////////////////////////////////////////////////////////////

#include <valarray>

#include "Math/RealVector.hpp"
#include "Math/RealMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// This class inverts a matrix using LU decomposition
/// @author Tiago Quintino
template < unsigned int SIZE >
struct LUInverterT {
public:

  /// Constructor
  LUInverterT ();

  /// Invert the given matrix a and put the result in x
  void invert (const RealMatrix& a, RealMatrix& x);

  /// Factorize the matrix
  void factorizeLU();

  /// Solve with forward and backward substitution
  void solveForwBack();

private: // data
  /// storage of indexes
  std::valarray<Uint> m_indx;
  /// temporary vector
  RealVector              m_tmp_col;
  /// temporary vector
  RealVector              m_vv;
  /// temporary copy of input matrix
  RealMatrix              m_a;

}; // end of class LUInverter

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#include "Math/LUInverterT.ci"

////////////////////////////////////////////////////////////////////////////////
#endif // CF_Math_LUInverterT_hh
