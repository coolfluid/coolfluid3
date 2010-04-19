#ifndef CF_Math_SVDInverter_hh
#define CF_Math_SVDInverter_hh

////////////////////////////////////////////////////////////////////////////////

#include "RealVector.hpp"
#include "MatrixInverter.hpp"
#include "Common/Exception.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// This class represents a solver that uses the SVD decomposition,
/// based on Numerical Recipes v3.01
/// A = U*S*V'  -->  pinv(A) = V*inv(S)*U'
/// U and V are orthogonal --> inv(U)=U' , inv(V)=V'
/// S is diagonal
/// @author Willem Deconinck
class Math_API SVDInverter : public MatrixInverter {
public:

  /// Constructor
  /// @param   nbRows   number of rows
  /// @param   nbCols   number of columns
  SVDInverter(const Uint& nbRows, const Uint& nbCols);

  /// @param    a   The matrix to apply SVD to
  SVDInverter(const RealMatrix& a);


  /// Default destructor
  ~SVDInverter();

  /// Invert the given matrix a and put the result in x
  /// @param    a   The matrix to invert
  /// @param    x   The inverted matrix
  void invert(const RealMatrix& a, RealMatrix& x);

  /// Invert the given matrix a and put the result in x
  /// @param    x   The inverted matrix
  void invert(RealMatrix& x);

  /// Solve A x = b for a vector x using the pseudoinverse of A
  /// as obtained by SVD. If positive, thresh is the threshold
  /// value below which singular values are considered as zero.
  /// If thresh is negative, a default based on expected roundoﬀ
  /// error is used.
  ///
  /// @param    a   The matrix of the system
  /// @param    b   The right hand side of the system
  /// @param    x   The solution of the system
  /// @param    tresh   treshold value below which singular values are considered as zero
void solve(const RealMatrix& a, const RealVector& b, RealVector& x, Real thresh = -1.);

  /// Solve a system A*x=b for x
  /// with multiple right hand sides
  ///
  /// @param    a   The matrix of the system
  /// @param    b   The right hand sides of the system
  /// @param    x   The solutions of the system
  /// @param    tresh   treshold value below which singular values are considered as zero
void solve(const RealMatrix& a, const RealMatrix& b, RealMatrix& x, Real thresh = -1.);

  /// Solve A x = b for a vector x using the pseudoinverse of A
  /// as obtained by SVD. If positive, thresh is the threshold
  /// value below which singular values are considered as zero.
  /// If thresh is negative, a default based on expected roundoﬀ
  /// error is used.
  ///
  /// @param    b   The right hand side of the system
  /// @param    x   The solution of the system
  /// @param    tresh   treshold value below which singular values are considered as zero
  void solve(const RealVector& b, RealVector& x, Real thresh = -1.);

  /// Solve a system A*x=b for x
  /// with multiple right hand sides
  ///
  /// @param    b   The right hand sides of the system
  /// @param    x   The solutions of the system
  /// @param    tresh   treshold value below which singular values are considered as zero
void solve(const RealMatrix& b, RealMatrix& x, Real thresh = -1.);

  /// Get the matrix U where A=U*S*V'
  /// @return   The matrix U
  RealMatrix getU() { return m_u;}

  /// Get the matrix V where A=U*S*V'
  /// @return   The matrix V
  RealMatrix getV() { return m_v;}

  /// Get the matrix S where A=U*S*V'
  /// @return   The singular values S (in vector)
  RealVector getS() { return m_s;}

  /// @param    tresh   treshold value below which singular values are considered as zero
  /// @return   The rank of the SVD problem
  Uint rank(Real thresh);

  /// @param    tresh   treshold value below which singular values are considered as zero
  /// @return   The nullity of the SVD problem
  Uint nullity(Real thresh);

  /// @param    tresh   treshold value below which singular values are considered as zero
  /// @return   The range of the SVD problem
  RealMatrix range(Real thresh);

  /// @param    tresh   treshold value below which singular values are considered as zero
  /// @return   The nullspace of the SVD problem
  RealMatrix nullspace(Real thresh);

  /// The inverse of the condition number
  /// @return   The inverse of the condition number
  Real inv_condition() {
  return (m_s[0] <= 0. || m_s[m_cols-1] <= 0.) ? 0. : m_s[m_cols-1]/m_s[0];
  }
  
  /// Singular Value Decomposition
  void decompose(const RealMatrix& a);

  /// Reorder singular values in descending order
  void reorder();

private: //helper functions

  Real pythag(const Real a, const Real b);

  /// Singular Value Decomposition
  void decompose();
  
private: //data

  /// The number of rows and columns of A=U*S*V'
  Uint                m_rows, m_cols;

  /// The matrices U and V
  RealMatrix             m_u, m_v;

  /// The diagonal matrix S (in vector)
  RealVector             m_s;

  /// Check to see if decomposed
  bool                   m_decomposed;

}; // end of class SVDInverter

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_SVDInverter_hh
