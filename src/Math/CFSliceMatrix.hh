#ifndef COOLFluiD_MathTools_CFSliceMatrix_hh
#define COOLFluiD_MathTools_CFSliceMatrix_hh

//////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>

#include "MathTools/MathChecks.hh"
#include "MathTools/ExprOp.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace MathTools {

    template <class T> class CFMatrix;

    template <class T> class CFSliceMatrix;

    template <class T> std::ostream& operator<< (std::ostream& out, const CFSliceMatrix<T>& A);

    template <class T> std::istream& operator>> (std::istream& in, CFSliceMatrix<T>& A);

    template <class T> bool operator== (const CFSliceMatrix<T>& A, const CFSliceMatrix<T>& B);

    template <class T> bool operator!= (const CFSliceMatrix<T>& A, const CFSliceMatrix<T>& B);

//////////////////////////////////////////////////////////////////////////////

///  Definition of a class Matrix for numerical applications that stores the
///  elements in a std::valarray but transparently gives access as a matrix.
///  The CFSliceMatrix is always oriented by rows. Consider generalizing it to both
///  orientations in the future.
/// @author Andrea Lani
/// @author Tiago Quintino
template<class T>
class CFSliceMatrix : public Expr<CFSliceMatrix<T>, T> {

public:

  /// Overloading of the stream operator "<<" for the output.
  /// "\n"ine introduced at the end of every line of the matrix.
  friend std::ostream& operator<< LTGT (std::ostream& out, const CFSliceMatrix<T>& A);

  /// Overloading of the stream operator ">>" for the input
  friend std::istream& operator>> LTGT (std::istream& in, CFSliceMatrix<T>& A);

  /// Overloading of the "==" operator.
  /// @return true if all elements are equal elementwise
  friend bool operator== LTGT (const CFSliceMatrix<T>& A,
    const CFSliceMatrix<T>& B);

  /// Overloading of the "!=" operator.
  /// @return true if all elements are different elementwise
  friend bool operator!= LTGT (const CFSliceMatrix<T>& A,
    const CFSliceMatrix<T>& B);

  /// Constructor
  CFSliceMatrix(const CFuint nRowMat,
  	const CFuint nColMat,
  	T *const start);

  /// Copy Constructor
  /// @param init object to copy from
  CFSliceMatrix(const CFSliceMatrix<T>& init);

  /// Overloading for operator= taking an expression as argument
#define SLICEMAT_EQ_OP(__op__)  					\
  template <class EXPR>  						\
  const CFSliceMatrix<T>& operator __op__ (const Expr<EXPR,T>& expr)  \
  {  								\
    const CFuint nm = size();  					\
    for (CFuint i = 0; i < nm; ++i) {  				\
      (*this)[i] __op__ expr.at(i);  				\
    }  								\
    return *this;  						\
}

SLICEMAT_EQ_OP(=)
SLICEMAT_EQ_OP(+=)
SLICEMAT_EQ_OP(-=)
SLICEMAT_EQ_OP(*=)
SLICEMAT_EQ_OP(/=)

#undef SLICEMAT_EQ_OP

/// Overloading for operator= taking a constant value as argument
#define SLICEMAT_EQ_OP_CONST(__op__)  			\
  const CFSliceMatrix<T>& operator __op__ (const T& value)  \
  {  						\
    const CFuint nm = size();  			\
for (CFuint i = 0; i < nm; ++i) {\
  (*this)[i] __op__ value;   \
}\
    return *this;\
}

SLICEMAT_EQ_OP_CONST(=)
SLICEMAT_EQ_OP_CONST(+=)
SLICEMAT_EQ_OP_CONST(-=)
SLICEMAT_EQ_OP_CONST(*=)

#undef SLICEMAT_EQ_OP_CONST

  /// Overloading of the assignment operator object to copy from
  const CFSliceMatrix& operator= (const CFSliceMatrix<T>& other);

  /// Overloading of "/="
  /// @param value of type T
  const CFSliceMatrix<T>& operator/= (const T& value)
  {
    cf_assert(std::abs(value) > std::numeric_limits<T>::epsilon());
    for (CFuint i = 0; i < _nRowSlice; ++i) {
      for (CFuint j = 0; j < _nColSlice; ++j) {
  (*this)(i,j) /= value;
      }
    }
    //1./value is dangerous : implicit conversion of T to CFreal
    return *this;
  }

  /// Default destructor
  ~CFSliceMatrix();

  /// Overloading of the operator"()" for assignment.
  T& operator() (CFuint i, CFuint j)
  {
    cf_assert(i < _nRowSlice);
    cf_assert(j < _nColSlice);
    return _start[i*_nColMat + j];
  }

  /// Overloading of the operator"()" (doesn't allow assignment)
  const T& operator() (CFuint i, CFuint j) const
  {
    cf_assert(i < _nRowSlice);
    cf_assert(j < _nColSlice);
    return _start[i*_nColMat + j];
  }

  /// Accessor used by the expression template wrapper class.
  T at (CFuint i) const
  {
    cf_assert(i < size());
    return operator()(i/_nColSlice, i%_nColSlice);
  }

  /// Overloading of the operator"[]" for assignment
  T& operator[] (CFuint i)
  {
    cf_assert(i < size());
    return operator()(i/_nColSlice, i%_nColSlice);
  }

  /// Overloading of the operator"()" (doesn't allow assignment)
  const T& operator[] (CFuint i) const
  {
    cf_assert(i < size());
    return operator()(i/_nColSlice, i%_nColSlice);
  }

  /// Gets the number of Rows.
  /// @return the number of rows in the matrix
  static CFuint nbRows()
  {
    return _nRowSlice;
  }

  /// Gets the number of Columns.
  /// @return the number of columns in the matrix
  static CFuint nbCols()
  {
    return _nColSlice;
  }

  /// Gets the size of the CFSliceMatrix
  /// @return CFuint with size of the std::valarray stored inside
  CFuint size() const
  {
    return _nRowSlice*_nColSlice;
  }

  /// Checks if CFSliceMatrix is null
  bool isNull() const { return ( _start == CFNULL ); }

  /// Get the maximum element
  T emax() const
  {
    const CFuint ns = _nRowSlice*_nColSlice;
    T result = _start[0];
    for (CFuint i = 1; i < ns; ++i) {
      if (_start[i] > result) { result = _start[i]; }
    }
    return result;
  }

  /// Get the minimum element
  T emin() const
  {
    const CFuint ns = _nRowSlice*_nColSlice;
    T result = _start[0];
    for (CFuint i = 1; i < ns; ++i) {
      if (_start[i] < result) {
  result = _start[i];
      }
    }
    return result;
  }

  /// Set the number of rows and cols of the slice
  static void setNbRowsCols(const CFuint nbRows,
          const CFuint nbCols)
  {
    _nRowSlice = nbRows;
    _nColSlice = nbCols;
  }

private:

  /// number of rows-columns in this slice
  CFuint _nRowMat;

  /// number of rows-columns in this slice
  CFuint _nColMat;

  /// pointer to the first element of the slice
  T* _start;

  /// size of the slice
  static CFuint _nRowSlice;

  /// size of the slice
  static CFuint _nColSlice;

};

//////////////////////////////////////////////////////////////////////////////

template<class T>
CFuint CFSliceMatrix<T>::_nRowSlice = 0;

template<class T>
CFuint CFSliceMatrix<T>::_nColSlice = 0;

//////////////////////////////////////////////////////////////////////////////

template <class T>
CFSliceMatrix<T>::CFSliceMatrix(const CFuint nRowMat,
        const CFuint nColMat,
        T *const start) :
  Expr<CFSliceMatrix<T>, T>(*this),
  _nRowMat(nRowMat),
  _nColMat(nColMat),
  _start(start)
{
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
CFSliceMatrix<T>::~CFSliceMatrix()
{
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
CFSliceMatrix<T>::CFSliceMatrix(const CFSliceMatrix<T>& orig) :
  Expr<CFSliceMatrix<T>, T>(*this),
  _nRowMat(orig._nRowMat),
  _nColMat(orig._nColMat),
  _start(orig._start)
{
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
const CFSliceMatrix<T>& CFSliceMatrix<T>::operator= (const CFSliceMatrix<T>& other)
{
  for (CFuint i = 0; i < _nRowSlice; ++i) {
    for (CFuint j = 0; j < _nColSlice; ++j) {
      (*this)(i,j) = other(i,j);
    }
  }
  return *this;
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
std::ostream& operator<< (std::ostream& out, const CFSliceMatrix<T>& v)
{
  const CFuint m = v.getNbRows();
  const CFuint n = v.getNbColumns();

  for (CFuint i = 0; i < m; ++i) {
    for (CFuint j = 0; j < n; ++j) {
      out << v(i,j) << " " ;
    }
  }
  return out;
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
std::istream& operator>> (std::istream& in, CFSliceMatrix<T>& v)
{
  const CFuint m = v.getNbRows();
  const CFuint n = v.getNbColumns();

  for (CFuint i = 0; i < m; ++i) {
    for (CFuint j = 0; j < n; ++j) {
      in >> v(i,j);
    }
  }

  return in;
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator== (const CFSliceMatrix<T>& m1,
      const CFSliceMatrix<T>& m2)
{
  if (m1._nRowSlice != m2._nRowSlice) return false;
  if (m1._nColSlice != m2._nColSlice) return false;

  for (CFuint i = 0; i < m1._nRowSlice; ++i) {
    for (CFuint j = 0; j < m1._nColSlice; ++j) {
      if (MathChecks::isNotEqual(m1(i,j), m2(i,j))) {
  return false;
      }
    }
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator!= (const CFSliceMatrix<T>& m1,
      const CFSliceMatrix<T>& m2)
{
  return !(m1 == m2);
}

//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

} // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_MathTools_CFSliceMatrix_hh
