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

template < unsigned int SIZE >
LUInverterT<SIZE>::LUInverterT() :
  m_indx(SIZE),
  m_tmp_col(SIZE),
  m_vv(SIZE),
  m_a(SIZE,SIZE,0.0)
{
}

////////////////////////////////////////////////////////////////////////////////

template < unsigned int SIZE >
void LUInverterT<SIZE>::invert(const RealMatrix& a, RealMatrix& x)
{
  m_a = a;
  factorizeLU();

  for (Uint j = 0;  j < SIZE; ++j) {
    m_tmp_col = 0.0;
    m_tmp_col[j] = 1.0;
    solveForwBack();
    for (Uint i = 0; i < SIZE; ++i) {
      x(i,j) = m_tmp_col[i];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

template < unsigned int SIZE >
void LUInverterT<SIZE>::factorizeLU()
{
  Uint imax = 0;
  Real big = 0.0;
  Real dum = 0.0;
  Real sum = 0.0;
  Real temp = 0.0;
  Real d = 1.0;  // no row permutation yet

  for (Uint i = 0; i < SIZE; ++i) {
    big = 0.0;
    for (Uint j = 0; j < SIZE; ++j) {
      if ((temp = std::abs(m_a(i,j))) > big) {
big = temp;
      }
    }
    cf_assert(std::abs(big) > Math::MathConsts::RealEps());
    m_vv[i] = 1.0/big;
  }
  for (Uint j = 0; j < SIZE; ++j) {
    for (Uint i = 0; i < j; ++i) {
      sum = m_a(i,j);
      for (Uint k = 0; k < i; ++k) {
sum -= m_a(i,k)*m_a(k,j);
      }
      m_a(i,j) = sum;
    }
    big = 0.0;

    for (Uint i = j; i < SIZE; ++i) {
      sum = m_a(i,j);
      for (Uint k = 0; k < j; ++k) {
sum -= m_a(i,k)*m_a(k,j);
      }
      m_a(i,j) = sum;
      if ((dum = m_vv[i]*std::abs(sum)) >= big) {
big = dum;
imax = i;
      }
    }
    if (j != imax) {
      for (Uint k = 0; k < SIZE; ++k) {
dum = m_a(imax,k);
m_a(imax,k) = m_a(j,k);
m_a(j,k) = dum;
      }
      d = -d;
      m_vv[imax] = m_vv[j];
    }
    m_indx[j] = imax;
    cf_assert(MathChecks::isNotZero(m_a(j,j)));
    if (j != SIZE) {
      dum = 1.0 / m_a(j,j);
      for (Uint i = j+1; i < SIZE; ++i) {
m_a(i,j) *= dum;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

template < unsigned int SIZE >
void LUInverterT<SIZE>::solveForwBack()
{
  int ip = 0;
  int ii = -1;
  Real sum = 0.0;

  for (Uint i = 0; i < SIZE; ++i) {
    ip = m_indx[i];
    sum = m_tmp_col[ip];
    m_tmp_col[ip] = m_tmp_col[i];
    if (ii >= 0) {
      for (Uint j = ii; j <= i-1; ++j) {
sum -= m_a(i,j)*m_tmp_col[j];
      }
    }
    else if (sum) {
      ii = i;
    }
    m_tmp_col[i] = sum;
  }
  for (int i = SIZE-1; i >= 0; --i) {
    sum = m_tmp_col[i];
    for (Uint j = i+1; j < SIZE; ++j) {
      sum -= m_a(i,j)*m_tmp_col[j];
    }
    m_tmp_col[i] = sum/m_a(i,i);
  }
}
////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_LUInverterT_hh
