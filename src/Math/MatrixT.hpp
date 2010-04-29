#ifndef CF_Math_MatrixT_hpp
#define CF_Math_MatrixT_hpp

////////////////////////////////////////////////////////////////////////////////

#include <ostream>

#include "Math/VectorT.hpp"
#include "Math/MatrixSliceT.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

    template <class T> class MatrixT;
    template <class T> std::ostream& operator<< (std::ostream& out, const MatrixT<T>& A);
    template <class T> std::istream& operator>> (std::istream& in, MatrixT<T>& A);
    template <class T> bool operator== (const MatrixT<T>& A, const MatrixT<T>& B);
    template <class T> bool operator!= (const MatrixT<T>& A, const MatrixT<T>& B);
    template <class T> void copy       (const MatrixT<T>& v1, MatrixT<T>& v2);

////////////////////////////////////////////////////////////////////////////////

/// Provides some functors applying binary functions.
/// @author Andrea Lani
#define BINARYOP(__name__,__op__) \
    template <class T> \
        class __name__ {\
        public: \
          static void op(T& a, const T& b) {a __op__ b;} \
        };

    BINARYOP(EqualBin,=)
    BINARYOP(AddBin,+=)
    BINARYOP(SubBin,-=)
    BINARYOP(MultBin,*=)
    BINARYOP(DivBin,/=)

#undef BINARYOP

////////////////////////////////////////////////////////////////////////////////

///  Definition of a class Matrix for numerical applications that stores the
///  elements in a std::valarray but transparently gives access as a matrix.
///  The MatrixT is always oriented by rows. Consider generalizing it to both
///  orientations in the future.
/// @author Andrea Lani
/// @author Tiago Quintino
template < typename T >
class MatrixT : public Expr<MatrixT<T>, T> {
public:

  /// Overloading of the stream operator "<<" for the output.
  /// "\n"ine introduced at the end of every line of the matrix.
  friend std::ostream& operator<< <> (std::ostream& out, const MatrixT<T>& A);

  /// Overloading of the stream operator ">>" for the input
  friend std::istream& operator>> <> (std::istream& in, MatrixT<T>& A);

  /// Overloading of the "==" operator.
  /// @return true if all elements are equal elementwise
  friend bool operator== <> (const MatrixT<T>& A, const MatrixT<T>& B);

  /// Overloading of the "!=" operator.
  /// @return true if all elements are different elementwise
  friend bool operator!= <> (const MatrixT<T>& A, const MatrixT<T>& B);

  /// Copy one VectorT into another one
  /// @pre v1.size() == v2.size()
  /// @param v1 source vector
  /// @param v2 destination vector
  friend void copy <> (const MatrixT<T>& orig, MatrixT<T>& dest);

  /// Default Constructor
  MatrixT() :  Expr<MatrixT<T>, T>(*this),
  m_owner(true), m_rows(0), m_cols(0), m_data(CFNULL) {}

  /// Constructor with initialization.
  /// @param mn number of rows
  /// @param nn number of columns
  /// @param value to initialize
  MatrixT(Uint mn, Uint nn, T value = T());

  /// Constructor by size and array initializer
  /// @param mn Uint with rows of matrix described in array
  /// @param nn Uint with columns of matrix described in array
  /// @param init T* array storing dynamical allocated memory
  MatrixT(bool owner, Uint mn, Uint nn, T* init);

  /// Copy Constructor
  /// @param init object to copy from
  MatrixT(const MatrixT<T>& init);

  /// Copy Constructor from an expression
  /// @param expr from which constructing the vector
  /// @param n    size of the vector to contruct
  /// @pre this works only for squared matrices
  /// @pre this should be used carefully
  template <class EXPR>
  MatrixT(const Expr<EXPR,T>& expr);

  /// Destructor
  ~MatrixT();

  /// This allows to reset the inner pointer of the MatrixT
  /// It must be used cautiously, only in case in which there is no ownership
  void resetPtr(T* ptr) {cf_assert(!m_owner);  m_data = ptr;}

  /// Overloading for operator= taking an expression as argument
#define MAT_EQ_OP(__op__)        \
  template <class EXPR>          \
  const MatrixT<T>& operator __op__ (const Expr<EXPR,T>& expr)  \
  {\
    const size_t nm = size();    \
    for (size_t i = 0; i < nm; ++i) {    \
  m_data[i] __op__ expr.at(i);\
}\
    return *this;\
}

MAT_EQ_OP(=)
MAT_EQ_OP(+=)
MAT_EQ_OP(-=)
MAT_EQ_OP(*=)
MAT_EQ_OP(/=)

#undef MAT_EQ_OP

/// Overloading for operator= taking a constant value as argument
#define MAT_EQ_OP_CONST(__op__) \
  const MatrixT<T>& operator __op__ (const T& value)  \
  {              \
    const size_t nm = size();      \
for (size_t i = 0; i < nm; ++i) {\
  m_data[i] __op__ value;\
}\
    return *this;\
}

MAT_EQ_OP_CONST(=)
MAT_EQ_OP_CONST(+=)
MAT_EQ_OP_CONST(-=)
MAT_EQ_OP_CONST(*=)

#undef MAT_EQ_OP_CONST

  /// Overloading of the assignment op=() operators for VectorT
#define MAT_ASSIGN_OP_VectorT(__op__)          \
const MatrixT<T>& operator __op__ (const VectorT<T>& diag) \
{                                                            \
  for (size_t i = 0; i < m_rows; ++i) { \
    (*this)(i,i) __op__ diag[i];     \
  }                                  \
  return *this;                      \
}

  MAT_ASSIGN_OP_VectorT(=)
  MAT_ASSIGN_OP_VectorT(+=)
  MAT_ASSIGN_OP_VectorT(-=)
  MAT_ASSIGN_OP_VectorT(*=)
  MAT_ASSIGN_OP_VectorT(/=)

#undef MAT_ASSIGN_OP_VectorT

  /// Fast implementation of matrix*matrix product (overloading of the general
  /// function taking the corresponding expression)
#define MAT_MAT_PROD(__op__,__prodFun__) \
const MatrixT<T>& operator __op__ (const Expr<Mult<MatrixT<T>,MatrixT<T>,T>,T>& expr) \
{\
__prodFun__ (expr.getData().ex1.getData(), expr.getData().ex2.getData()); \
return *this; \
}

MAT_MAT_PROD(=,prod< EqualBin<T> >)
MAT_MAT_PROD(+=,prod< AddBin<T> >)
MAT_MAT_PROD(-=,prod< SubBin<T> >)

#undef MAT_MAT_PROD

  /// Fast implementation of matrix*(vector*matrix) product (overloading of the general
  /// function taking the corresponding expression)
#define MAT_VEC_MAT_PROD(__op__,__prodFun__) \
const MatrixT<T>& operator __op__ (const Expr<Mult<MatrixT<T>,Mult<VectorT<T>,MatrixT<T>,T>,T>,T>& expr) \
{\
__prodFun__ (expr.getData().ex1.getData(), \
expr.getData().ex2.getData().ex1.getData(), \
expr.getData().ex2.getData().ex2.getData()); \
return *this; \
}

  MAT_VEC_MAT_PROD(=,prod< EqualBin<T> >)
  MAT_VEC_MAT_PROD(+=,prod< AddBin<T> >)
  MAT_VEC_MAT_PROD(-=,prod< SubBin<T> >)

#undef MAT_VEC_MAT_PROD

  /// Overloading of the assignment operator "=".
  /// If the sizes don't match, assignee is erased and resized.
  /// @param B object to equal
  const MatrixT<T>& operator= (const MatrixT<T>& B);

  /// Overloading of "/="
  /// @param value of type T
  const MatrixT<T>& operator/= (const T& value)
  {
    cf_assert(std::abs(value) > std::numeric_limits<T>::epsilon());
    const size_t msize = size();
    for (size_t i = 0; i < msize; ++i) {
      m_data[i] /= value;
    }

    //1./value is dangerous : implicit conversion of T to Real
    return *this;
  }

  /// Accessor used by the expression template wrapper class
  T at (Uint i) const { return m_data[i]; }

  /// Overloading of the operator"()" for assignment.
  T& operator() (Uint, Uint);

  /// Overloading of the operator"()" (doesn't allow assignment)
  T operator() (Uint, Uint) const;

  /// Apply a LU factorization of a given matrix
  /// @param mat matrix
  void factorizeLU();

  /// Returns the transposes of the object matrix.
  /// The object remains untransposed.
  /// @param result transposed matrix
  void transpose(MatrixT<T>& result) const;

  /// Gets the number of Rows.
  /// @return the number of rows in the matrix
  Uint nbRows() const { return m_rows; }

  /// Gets the number of Columns.
  /// @return the number of columns in the matrix
  Uint nbCols() const { return m_cols; }

  /// Function returning a slice of this MatrixT
  MatrixSliceT<T> slice (const Uint iStart, const Uint jStart)
  {
    return MatrixSliceT<T>(m_rows, m_cols, &m_data[iStart*m_cols + jStart]);
  }

  /// Gets a VectorT with the copy of the row of the MatrixT.
  /// @param iRow  number of the row
  /// @return the specified row of the matrix.
  VectorT<T> getRow(const Uint iRow) const;

  /// Gets a  VectorT with the copy of the column of the MatrixT.
  /// @param iCol  number of the column
  /// @return the specified column of the matrix.
  VectorT<T> getColumn(const Uint iCol) const;

  /// Puts in a supplied VectorT, the copy of the row of the MatrixT.
  /// @param iRow  number of the row
  void putRow(const Uint iRow, VectorT<T>& v) const;

  /// Puts in a supplied VectorT, the copy of the column of the MatrixT.
  /// @param iCol  number of the column
  void putColumn(const Uint iCol, VectorT<T>& v) const;

  /// Set a row of the matrix.
  /// @param row VectorT with the row to set
  /// @param iRow  number of the row
  void setRow(const VectorT<T>& row, const Uint iRow);

  /// Set a column of the matrix.
  /// @param col VectorT with the column to set
  /// @param iCol  number of the column
  void setColumn(const VectorT<T>& col, const Uint iCol);

  /// Add a row to the matrix.
  /// @param row VectorT with the row to set
  /// @param iRow  number of the row
  void addRow(const VectorT<T>& row, const Uint iRow);

  /// Add a column to the matrix.
  /// @param col VectorT with the column to set
  /// @param iCol  number of the column
  void addColumn(const VectorT<T>& col, const Uint iCol);

  /// Set a column of the given matrix in the current one.
  /// @param col VectorT with the column to set
  /// @param iCol  number of the column
  void setColumn(Uint iCol1, const MatrixT<T>& m1, Uint iCol2)
  {
    for (size_t i = 0; i < m_rows; ++i) {
      const size_t ni = m_cols*i;
      m_data[iCol2 + ni] = m1[ni + iCol1];
    }
  }

  /// Get a VectorT with all data of the matrix in a row oriented disposition.
  /// @return VectorT with data of the matrix.
  VectorT<T> getVector() const;

  /// Sums the rows of the MatrixT into a VectorT
  /// @return  VectorT<T> with the row summation of MatrixT.
  VectorT<T> sumRows() const;

  /// Sums the columns of the MatrixT into a VectorT
  /// @return  VectorT<T> with the column sumation of MatrixT.
  VectorT<T> sumColumns() const;

  /// Sums the squares of the rows of the MatrixT into a VectorT
  /// @return  VectorT<T> with the row square sumation of MatrixT.
  VectorT<T> sumSqRows() const;

  /// Sums the squares of the columns of the MatrixT into a VectorT
  /// @return  VectorT<T> with the column square sumation of MatrixT.
  VectorT<T> sumSqColumns() const;

  /// Sums the rows into the passed vector
  void sumRowsTo (VectorT<T>& v) const;
  /// Sums the columns into the passed vector
  void sumColumnsTo (VectorT<T>& v) const;
  /// Copies the ith row into the passed vector
  void copyRowTo(VectorT<T>& v, const Uint& i) const;
  /// Copies the jth column into the passed vector
  void copyColumnTo(VectorT<T>& v, const Uint& j) const;
  /// Copies the diagonal into the passed vector
  void copyDiagonalTo(VectorT<T>& v, const Uint begin, const Uint end);

  /// Calculates the trace of the MatrixT
  /// @return T with the trace of the MatrixT
  T trace() const;

  /// Clears and resizes the matrix
  /// @param rows number of rows for new size
  /// @param cols number of columns for new size
  void resize(const Uint rows, const Uint cols, bool owner = true);

  /// Gets the size of the MatrixT
  /// @return Uint with size of the std::valarray stored inside
  Uint size() const { return m_rows*m_cols; }

  /// Checks if MatrixT is null
  bool isNull() const;

  /// Checks if MatrixT is a square matrix.
  /// @return true if square, false otherwise
  bool isSquare() const;

  /// Checks if MatrixT is a symmetric matrix.
  /// @return true if symmetric, false otherwise
  bool isSymmetric() const;

  /// Calculate the determinant of a 2*2 matrix
  T determ2() const;

  /// Calculate the determinant of a 3*3 matrix
  T determ3() const;

  /// Calculate the determinant of a 4*4 matrix
  T determ4() const;

  /// Invert a diagonal matrix
  void invertDiag(MatrixT<T>& result) const;

  /// Get the maximum element
  T emax() const;

  /// Get the minimum element
  T emin() const;

  /// Overloading of the operator"[]" for assignment
  /// @return m_data[i]
  T& operator[] (Uint i);

  /// Overloading of the operator"[]"
  /// @return m_data[i]
  T operator[] (Uint i) const;

private: // helper function

  /// Fast implementation for matrix*matrix (impossible with conventional "elementwise"
  /// expression templates).
  template <class BINOP>
  void prod(const MatrixT<T>& A, const MatrixT<T>& B);

  /// Fast implementation for matrix*(vector*matrix) (impossible with conventional "elementwise"
  /// expression templates).
  template <class BINOP>
  void prod(const MatrixT<T>& A, const VectorT<T>& v, const MatrixT<T>& B);

  /// Deletes the allocated memory in case of ownership
  void release_mem ();

  /// Allocates the memory for the size
  void alloc_mem () {
    cf_assert(m_owner && m_data == CFNULL);
    if (m_rows*m_cols > 0) m_data = new T [m_rows*m_cols];
  }

private: // data

  /// indicates ownership of the memory
  /// is false if memory was allocated outside
  bool m_owner;

  /// number of rows in the matrix
  Uint m_rows;

  /// number of columns in the matrix
  Uint m_cols;

  /// storage of the data
  T* cf_restrict m_data;

}; // end class MatrixT

////////////////////////////////////////////////////////////////////////////////

template<class T>
MatrixT<T>::MatrixT(Uint mn,
          Uint nn,
          T value) :
  Expr<MatrixT<T>, T>(*this),
  m_owner (true), m_rows(mn), m_cols(nn), m_data(CFNULL)
{
  alloc_mem();
  operator= (value);
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
MatrixT<T>::MatrixT(bool owner,
          Uint mn,
          Uint nn,
          T* init) :
  Expr<MatrixT<T>, T>(*this),
  m_owner(owner), m_rows(mn), m_cols(nn), m_data(CFNULL)
{
  if (m_owner) {
    alloc_mem();
    const size_t msize = m_rows*m_cols;
    for(size_t i = 0; i < msize; ++i) {
      m_data[i] = init[i];
    }
  }
  else {
    m_data = init;
  }
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
MatrixT<T>::MatrixT(const MatrixT<T>& init) :
  Expr<MatrixT<T>, T>(*this),
  m_owner (true),
  m_rows(init.m_rows),
  m_cols(init.m_cols),
  m_data(CFNULL)
{
  alloc_mem();
  copy(init,*this);
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
template <class EXPR>
MatrixT<T>::MatrixT(const Expr<EXPR,T>& expr) :
  Expr<MatrixT<T>, T>(*this),
  m_owner(true)
{
  const Uint sizeN = static_cast<Uint>(std::sqrt(static_cast<T>(expr.size())));
  m_rows = sizeN;
  m_cols = sizeN;
  m_data = CFNULL;

  alloc_mem();
  const size_t msize = size();
  for (size_t i = 0; i < msize; ++i) {
    m_data[i] = expr.at(i);
  }
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
MatrixT<T>::~MatrixT()
{
  release_mem();
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
void MatrixT<T>::release_mem ()
{
  if (m_owner && m_data != CFNULL)
  {
    delete [] m_data;
    m_data = CFNULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
const MatrixT<T>& MatrixT<T>::operator= (const MatrixT<T>& B)
{
  cf_assert(&B != this);

  if (m_rows != B.m_rows || m_cols != B.m_cols) {
    if (m_owner) {
      release_mem();
    }

    m_rows = B.m_rows;
    m_cols = B.m_cols;

    if (m_owner) {
      alloc_mem();
    }
  }

  const size_t msize = size();
  for (size_t i = 0; i < msize; ++i) {
    m_data[i] = B.m_data[i];
  }

  return *this;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T& MatrixT<T>::operator()(Uint i, Uint j)
{
  cf_assert(i < m_rows);
  cf_assert(j < m_cols);
  return m_data[i*m_cols+j];
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T MatrixT<T>::operator()(Uint i, Uint j) const
{
  cf_assert(i < m_rows);
  cf_assert(j < m_cols);
  return m_data[i*m_cols+j];
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T& MatrixT<T>::operator[](Uint i)
{
  cf_assert(i < size());
  return m_data[i];
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T MatrixT<T>::operator[](Uint i) const
{
  cf_assert(i < size());
  return m_data[i];
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T MatrixT<T>::determ2() const
{
  cf_assert(m_rows == 2);
  cf_assert(m_cols == 2);
  return m_data[0]*m_data[3] - m_data[1]*m_data[2];
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
T MatrixT<T>::determ4() const
{
  cf_assert(m_rows == 4);
  cf_assert(m_cols == 4);

  T d00 = m_data[0];
  T d01 = m_data[1];
  T d02 = m_data[2];
  T d03 = m_data[3];
  T d04 = m_data[4];
  T d05 = m_data[5];
  T d06 = m_data[6];
  T d07 = m_data[7];

  T d10d15_d14d11 = m_data[10]*m_data[15] - m_data[14]*m_data[11];
  T d09d15_d13d11 = m_data[ 9]*m_data[15] - m_data[13]*m_data[11];
  T d09d14_d13d10 = m_data[ 9]*m_data[14] - m_data[13]*m_data[10];
  T d08d15_d12d11 = m_data[ 8]*m_data[15] - m_data[12]*m_data[11];
  T d08d13_d12d09 = m_data[ 8]*m_data[13] - m_data[12]*m_data[ 9];
  T d08d14_d12d10 = m_data[ 8]*m_data[14] - m_data[12]*m_data[10];

  return  d00*(d05*(d10d15_d14d11) - d06*(d09d15_d13d11) + d07*(d09d14_d13d10))
        - d01*(d04*(d10d15_d14d11) - d06*(d08d15_d12d11) + d07*(d08d14_d12d10))
        + d02*(d04*(d09d15_d13d11) - d05*(d08d15_d12d11) + d07*(d08d13_d12d09))
        - d03*(d04*(d09d14_d13d10) - d05*(d08d14_d12d10) + d06*(d08d13_d12d09));
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T MatrixT<T>::determ3() const
{
  cf_assert(m_rows == 3);
  cf_assert(m_cols == 3);

  return m_data[0]*(m_data[4]*m_data[8] - m_data[5]*m_data[7]) -
         m_data[1]*(m_data[3]*m_data[8] - m_data[5]*m_data[6]) +
         m_data[2]*(m_data[3]*m_data[7] - m_data[4]*m_data[6]);
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void MatrixT<T>::invertDiag(MatrixT<T>& result) const
{
  T temp = T();
  for (size_t i = 0; i < m_cols; ++i) {
    temp = (*this)(i,i);
    cf_assert(MathChecks::isNotZero(temp));
    result(i,i) = 1./temp;
  }
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void MatrixT<T>::resize(const Uint rows,
    const Uint cols,
    bool owner)
{
  m_owner = owner;
  m_rows = rows;
  m_cols = cols;

  if (m_owner) {
    release_mem();
    alloc_mem();
    operator= (T());
  }
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
VectorT<T> MatrixT<T>::sumRows() const
{
  VectorT<T> temp(m_rows);
  T sum;
  for (size_t i = 0; i < m_rows; ++i) {
    sum = T();
    for (size_t j = 0; j < m_cols; ++j) sum += (*this)(i,j);
    temp[i] = sum;
    }
  return temp;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
VectorT<T> MatrixT<T>::sumColumns() const
{
  VectorT<T> temp(m_cols);
  T sum;
  for (size_t i = 0; i < m_cols; ++i) {
    sum = T();
    for (size_t j = 0; j < m_rows; ++j) sum += (*this)(i,j);
    temp[i] = sum;
  }
  return temp;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
VectorT<T> MatrixT<T>::sumSqRows() const
{
  VectorT<T> temp(m_rows);
  T sum;
  for (size_t i = 0; i < m_rows; ++i) {
    sum = T();
    for (size_t j = 0; j < m_cols; ++j) sum += (*this)(i,j) * (*this)(i,j);
    temp[i] = sum;
  }
  return temp;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
VectorT<T> MatrixT<T>::sumSqColumns() const
{
  VectorT<T> temp(m_cols);
  T sum;
  for (size_t i = 0; i < m_cols; ++i) {
    sum = T();
    for (size_t j = 0; j < m_rows; ++j) sum += (*this)(i,j) * (*this)(i,j);
    temp[i] = sum;
  }
  return temp;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline VectorT<T> MatrixT<T>::getRow(const Uint iRow) const
{
  cf_assert(iRow < m_rows);
  VectorT<T> row(m_cols);
  putRow(iRow,row);
  return row;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline VectorT<T> MatrixT<T>::getColumn(const Uint iCol) const
{
  cf_assert(iCol < m_cols);
  VectorT<T> col(m_rows);
  putColumn(iCol,col);
  return col;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline VectorT<T> MatrixT<T>::getVector() const
{
  const size_t msize = size();
  VectorT<T> all(msize);
  for(size_t i = 0; i < msize; ++i)
    all[i] = m_data[i];
  return all;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void MatrixT<T>::putRow(const Uint iRow, VectorT<T>& v) const
{
  cf_assert(iRow < m_rows);
  cf_assert(v.size() == m_cols);
  for (size_t i = 0; i < m_cols; ++i)
    v[i] = m_data[iRow*m_cols + i];
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void MatrixT<T>::putColumn(const Uint iCol, VectorT<T>& v) const
{
  cf_assert(iCol < m_cols);
  cf_assert(v.size() == m_rows);
  for (size_t i = 0; i < m_rows; ++i)
    v[i] = m_data[m_cols*i + iCol];
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void MatrixT<T>::setRow(const VectorT<T>& row, const Uint iRow)
{
  cf_assert(row.size() == m_cols);
  size_t istart = iRow*m_cols;
  for (size_t i = 0; i < m_cols; ++i, ++istart) {
    m_data[istart] = row[i];
  }
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void MatrixT<T>::setColumn(const VectorT<T>& col, const Uint iCol)
{
  cf_assert(col.size() == m_rows);
  for (size_t i = 0; i < m_rows; ++i) {
    m_data[iCol+m_cols*i] = col[i];
  }
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void MatrixT<T>::addRow(const VectorT<T>& row, const Uint iRow)
{
  cf_assert(row.size() == m_cols);
  size_t istart = iRow*m_cols;
  for (size_t i = 0; i < m_cols; ++i, ++istart) {
    m_data[istart] += row[i];
  }
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void MatrixT<T>::addColumn(const VectorT<T>& col, const Uint iCol)
{
  cf_assert(col.size() == m_rows);
  for (size_t i = 0; i < m_rows; ++i) {
    m_data[iCol+m_cols*i] += col[i];
  }
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline bool MatrixT<T>::isNull() const
{
  const size_t msize = size();
  for (size_t i = 0; i < msize; ++i) {
    if (MathChecks::isNotZero(m_data[i])) return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline bool MatrixT<T>::isSquare() const
{
  return m_rows == m_cols;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
bool MatrixT<T>::isSymmetric() const
{
  if (m_rows != m_cols)
    return false;
  for (size_t i = 1; i < m_cols; ++i){
    for (size_t j = 0; j < i; ++j){
      if (m_data[i*m_cols + j] != m_data[j*m_cols + i])
  return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T MatrixT<T>::trace() const
{
  T traceRes = 0;
  size_t n = std::min( m_rows , m_cols );
  for (size_t i = 0; i < n; ++i) traceRes += m_data[i*m_cols + i];
  return traceRes;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
inline void MatrixT<T>::transpose(MatrixT<T>& result) const
{
  for (size_t i = 0; i < m_cols; ++i) {
    for (size_t j = 0; j < m_rows; ++j) {
      result(i,j) = m_data[j*m_cols + i];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
std::ostream& operator<< (std::ostream& out, const MatrixT<T>& A)
{
  for (size_t i = 0; i < A.m_rows; ++i) {
    for (size_t j = 0; j < A.m_cols; ++j) {
      out << A(i,j) << " " ;
      }
    out << "\n";
  }
  return out;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
std::istream& operator>> (std::istream& in, MatrixT<T>& A)
{
  for (size_t i = 0; i < A.m_rows; ++i) {
    for (size_t j = 0; j < A.m_cols; ++j) {
      in >> A(i,j);
    }
  }
  return in;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator== (const MatrixT<T>& A, const MatrixT<T>& B)
{
  cf_assert(A.size() == B.size());
  for(size_t i = 0; i < A.size(); ++i) {
    if (A.m_data[i] != B.m_data[i]) {
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator!= (const MatrixT<T>& A, const MatrixT<T>& B)
{
  return !(A == B);
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
template <class BINOP>
void MatrixT<T>::prod(const MatrixT<T>& A, const MatrixT<T>& B)
{
  cf_assert(B.m_rows == A.m_cols);

    const size_t nc = B.m_cols;
    const size_t m = A.m_rows;
    const size_t n = A.m_cols;

    for (size_t i = 0; i < m; ++i) {
      const size_t jstart = i*n;
      const size_t jmax = jstart + n;
      const size_t kstart = i*nc;
      size_t count = 0;
      size_t k = 0;
      while(count < nc) {
        T sum = T();
        for(size_t j = jstart; j < jmax; ++j) {
          sum += A.m_data[j] * B.m_data[k];
          k += nc;
        }
        BINOP::op(m_data[kstart + count], sum);
        k = ++count;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
template <class BINOP>
void MatrixT<T>::prod(const MatrixT<T>& A, const VectorT<T>& d,
                       const MatrixT<T>& B)
{
  cf_assert(B.m_rows == A.m_cols);
  cf_assert(d.size() == B.m_rows);

  const size_t nc = B.m_cols;
  const size_t m = A.m_rows;
  const size_t n = A.m_cols;

  for (size_t i = 0; i < m; ++i) {
    const size_t jstart = i*n;
    const size_t jmax = jstart + n;
    const size_t kstart = i*nc;
    size_t count = 0;
    size_t k = 0;
    while(count < nc) {
      T sum = 0;
      for(size_t j = jstart; j < jmax; ++j) {
         sum += A.m_data[j]*B.m_data[k]*d[j-jstart];
         k += nc;
      }
      BINOP::op(m_data[kstart + count], sum);
      k = ++count;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
void MatrixT<T>::sumRowsTo(VectorT<T>& v) const
{
  const size_t m = nbRows();
  const size_t n = nbCols();
  cf_assert(v.size() == m);
  for (size_t i = 0; i < m; ++i) {
    T sum = T();
    for (size_t j = 0; j < n; ++j) {
      sum += (*this)(i,j);
    }
    v[i] = sum;
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
void MatrixT<T>::sumColumnsTo(VectorT<T>& v) const
{
  const size_t m = nbRows();
  const size_t n = nbCols();
  cf_assert(size() == n);
  for (size_t i = 0; i < n; ++i)
  {
    T sum = T();
    for (size_t j = 0; j < m; ++j) {
      sum += (*this)(i,j);
    }
    v[i] = sum;
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline
void MatrixT<T>::copyRowTo(VectorT<T>& v, const Uint& i) const
{
  cf_assert(size() == nbCols());
  const size_t n = nbCols();
  for (size_t j = 0; j < n; ++j) {
    v[j] = (*this)(i,j);
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline
void MatrixT<T>::copyColumnTo(VectorT<T>& v, const Uint& j) const
{
  cf_assert(size() == nbRows());
  const size_t m = nbRows();
  for (size_t i = 0; i < m; ++i) {
    v[i] = (*this)(i,j);
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline
void MatrixT<T>::copyDiagonalTo(VectorT<T>& v, const Uint begin, const Uint end)
{
  cf_assert( begin <= end );
  cf_assert( end < std::min<Uint> ( nbRows(), nbCols() ));
  cf_assert( size() == end - begin + 1);

  for (size_t i = begin; i < end; ++i) {
    v[i] = (*this)(i,i);
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline void MatrixT<T>::factorizeLU()
{
  // actual LU factorization
  // loop over the diagonal elements
  const size_t size = nbRows();
  const size_t resSizeM1 = size - 1;
  for (size_t iDiag = 0; iDiag < resSizeM1; ++iDiag) {
    const Real invDiag = 1.0/(*this)(iDiag,iDiag);
    for (size_t iRow = iDiag+1; iRow < size; ++iRow) {
      const Real factor = (*this)(iRow,iDiag)*invDiag;
      (*this)(iRow,iDiag) = factor;// L matrix
      for (size_t iCol = iDiag+1; iCol < size; ++iCol) {
        (*this)(iRow,iCol) -= factor*(*this)(iDiag,iCol);
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

template <class T>
void copy (const MatrixT<T>& orig, MatrixT<T>& dest)
{
  cf_assert(orig.size() == dest.size());
  const size_t size = orig.size();
  for (size_t i = 0; i < size; ++i)
    dest.m_data[i] = orig.m_data[i];
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline T MatrixT<T>::emax () const
{
  const size_t msize = this->size();
  cf_assert (msize > 0);
  T rmax = m_data[0];
  for (size_t i = 1; i < msize; ++i)
    rmax = std::max(m_data[i],rmax);
  return rmax;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline T MatrixT<T>::emin () const
{
  const size_t msize = this->size();
  cf_assert (msize > 0);
  T rmin = m_data[0];
  for (size_t i = 1; i < msize; ++i)
    rmin = std::min(m_data[i],rmin);
  return rmin;
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_MatrixT_hpp
