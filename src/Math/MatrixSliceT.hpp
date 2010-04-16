#ifndef CF_Math_MatrixSliceT_hh
#define CF_Math_MatrixSliceT_hh

////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>

#include "Math/MathChecks.hpp"
#include "Math/ExprOp.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

    template <class T> class MatrixT;

    template <class T> class MatrixSliceT;

    template <class T> std::ostream& operator<< (std::ostream& out, const MatrixSliceT<T>& A);

    template <class T> std::istream& operator>> (std::istream& in, MatrixSliceT<T>& A);

    template <class T> bool operator== (const MatrixSliceT<T>& A, const MatrixSliceT<T>& B);

    template <class T> bool operator!= (const MatrixSliceT<T>& A, const MatrixSliceT<T>& B);

////////////////////////////////////////////////////////////////////////////////

///  Definition of a class Matrix for numerical applications that stores the
///  elements in a std::valarray but transparently gives access as a matrix.
///  The MatrixSliceT is always oriented by rows. Consider generalizing it to both
///  orientations in the future.
/// @author Andrea Lani
/// @author Tiago Quintino
template<class T>
class MatrixSliceT : public Expr<MatrixSliceT<T>, T> {

public:

  /// Overloading of the stream operator "<<" for the output.
  /// "\n"ine introduced at the end of every line of the matrix.
  friend std::ostream& operator<< <> (std::ostream& out, const MatrixSliceT<T>& A);

  /// Overloading of the stream operator ">>" for the input
  friend std::istream& operator>> <> (std::istream& in, MatrixSliceT<T>& A);

  /// Overloading of the "==" operator.
  /// @return true if all elements are equal elementwise
  friend bool operator== <> (const MatrixSliceT<T>& A,
    const MatrixSliceT<T>& B);

  /// Overloading of the "!=" operator.
  /// @return true if all elements are different elementwise
  friend bool operator!= <> (const MatrixSliceT<T>& A,
    const MatrixSliceT<T>& B);

  /// Constructor
  MatrixSliceT(const Uint nRowMat,
  const Uint nColMat,
  T *const start);

  /// Copy Constructor
  /// @param init object to copy from
  MatrixSliceT(const MatrixSliceT<T>& init);

  /// Overloading for operator= taking an expression as argument
#define SLICEMAT_EQ_OP(__op__)            \
  template <class EXPR>              \
  const MatrixSliceT<T>& operator __op__ (const Expr<EXPR,T>& expr)  \
  {                  \
    const Uint nm = size();            \
    for (Uint i = 0; i < nm; ++i) {          \
      (*this)[i] __op__ expr.at(i);          \
    }                  \
    return *this;              \
}

SLICEMAT_EQ_OP(=)
SLICEMAT_EQ_OP(+=)
SLICEMAT_EQ_OP(-=)
SLICEMAT_EQ_OP(*=)
SLICEMAT_EQ_OP(/=)

#undef SLICEMAT_EQ_OP

/// Overloading for operator= taking a constant value as argument
#define SLICEMAT_EQ_OP_CONST(__op__)        \
  const MatrixSliceT<T>& operator __op__ (const T& value)  \
  {              \
    const Uint nm = size();        \
for (Uint i = 0; i < nm; ++i) {\
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
  const MatrixSliceT& operator= (const MatrixSliceT<T>& other);

  /// Overloading of "/="
  /// @param value of type T
  const MatrixSliceT<T>& operator/= (const T& value)
  {
    cf_assert(std::abs(value) > std::numeric_limits<T>::epsilon());
    for (Uint i = 0; i < _nRowSlice; ++i) {
      for (Uint j = 0; j < _nColSlice; ++j) {
  (*this)(i,j) /= value;
      }
    }
    //1./value is dangerous : implicit conversion of T to Real
    return *this;
  }

  /// Default destructor
  ~MatrixSliceT();

  /// Overloading of the operator"()" for assignment.
  T& operator() (Uint i, Uint j)
  {
    cf_assert(i < _nRowSlice);
    cf_assert(j < _nColSlice);
    return _start[i*_nColMat + j];
  }

  /// Overloading of the operator"()" (doesn't allow assignment)
  const T& operator() (Uint i, Uint j) const
  {
    cf_assert(i < _nRowSlice);
    cf_assert(j < _nColSlice);
    return _start[i*_nColMat + j];
  }

  /// Accessor used by the expression template wrapper class.
  T at (Uint i) const
  {
    cf_assert(i < size());
    return operator()(i/_nColSlice, i%_nColSlice);
  }

  /// Overloading of the operator"[]" for assignment
  T& operator[] (Uint i)
  {
    cf_assert(i < size());
    return operator()(i/_nColSlice, i%_nColSlice);
  }

  /// Overloading of the operator"()" (doesn't allow assignment)
  const T& operator[] (Uint i) const
  {
    cf_assert(i < size());
    return operator()(i/_nColSlice, i%_nColSlice);
  }

  /// Gets the number of Rows.
  /// @return the number of rows in the matrix
  static Uint nbRows()
  {
    return _nRowSlice;
  }

  /// Gets the number of Columns.
  /// @return the number of columns in the matrix
  static Uint nbCols()
  {
    return _nColSlice;
  }

  /// Gets the size of the MatrixSliceT
  /// @return Uint with size of the std::valarray stored inside
  Uint size() const
  {
    return _nRowSlice*_nColSlice;
  }

  /// Checks if MatrixSliceT is null
  bool isNull() const { return ( _start == CFNULL ); }

  /// Get the maximum element
  T emax() const
  {
    const Uint ns = _nRowSlice*_nColSlice;
    T result = _start[0];
    for (Uint i = 1; i < ns; ++i) {
      if (_start[i] > result) { result = _start[i]; }
    }
    return result;
  }

  /// Get the minimum element
  T emin() const
  {
    const Uint ns = _nRowSlice*_nColSlice;
    T result = _start[0];
    for (Uint i = 1; i < ns; ++i) {
      if (_start[i] < result) {
  result = _start[i];
      }
    }
    return result;
  }

  /// Set the number of rows and cols of the slice
  static void setNbRowsCols(const Uint nbRows,
          const Uint nbCols)
  {
    _nRowSlice = nbRows;
    _nColSlice = nbCols;
  }

private:

  /// number of rows-columns in this slice
  Uint _nRowMat;

  /// number of rows-columns in this slice
  Uint _nColMat;

  /// pointer to the first element of the slice
  T* _start;

  /// size of the slice
  static Uint _nRowSlice;

  /// size of the slice
  static Uint _nColSlice;

};

////////////////////////////////////////////////////////////////////////////////

template<class T>
Uint MatrixSliceT<T>::_nRowSlice = 0;

template<class T>
Uint MatrixSliceT<T>::_nColSlice = 0;

////////////////////////////////////////////////////////////////////////////////

template <class T>
MatrixSliceT<T>::MatrixSliceT(const Uint nRowMat,
        const Uint nColMat,
        T *const start) :
  Expr<MatrixSliceT<T>, T>(*this),
  _nRowMat(nRowMat),
  _nColMat(nColMat),
  _start(start)
{
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
MatrixSliceT<T>::~MatrixSliceT()
{
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
MatrixSliceT<T>::MatrixSliceT(const MatrixSliceT<T>& orig) :
  Expr<MatrixSliceT<T>, T>(*this),
  _nRowMat(orig._nRowMat),
  _nColMat(orig._nColMat),
  _start(orig._start)
{
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
const MatrixSliceT<T>& MatrixSliceT<T>::operator= (const MatrixSliceT<T>& other)
{
  for (Uint i = 0; i < _nRowSlice; ++i) {
    for (Uint j = 0; j < _nColSlice; ++j) {
      (*this)(i,j) = other(i,j);
    }
  }
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
std::ostream& operator<< (std::ostream& out, const MatrixSliceT<T>& v)
{
  const Uint m = v.getNbRows();
  const Uint n = v.getNbColumns();

  for (Uint i = 0; i < m; ++i) {
    for (Uint j = 0; j < n; ++j) {
      out << v(i,j) << " " ;
    }
  }
  return out;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
std::istream& operator>> (std::istream& in, MatrixSliceT<T>& v)
{
  const Uint m = v.getNbRows();
  const Uint n = v.getNbColumns();

  for (Uint i = 0; i < m; ++i) {
    for (Uint j = 0; j < n; ++j) {
      in >> v(i,j);
    }
  }

  return in;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator== (const MatrixSliceT<T>& m1,
      const MatrixSliceT<T>& m2)
{
  if (m1._nRowSlice != m2._nRowSlice) return false;
  if (m1._nColSlice != m2._nColSlice) return false;

  for (Uint i = 0; i < m1._nRowSlice; ++i) {
    for (Uint j = 0; j < m1._nColSlice; ++j) {
      if (MathChecks::isNotEqual(m1(i,j), m2(i,j))) {
  return false;
      }
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator!= (const MatrixSliceT<T>& m1,
      const MatrixSliceT<T>& m2)
{
  return !(m1 == m2);
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_MatrixSliceT_hh
