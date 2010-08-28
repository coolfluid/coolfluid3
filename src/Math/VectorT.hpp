#ifndef CF_Math_VectorT_hpp
#define CF_Math_VectorT_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/MathChecks.hpp"
#include "Math/MathFunctions.hpp"
#include "Math/ExprOp.hpp"
#include "Math/VectorSliceT.hpp"
#include "Common/BoostArray.hpp"
#include "Math/Math.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

    template <class T> class VectorT;
    template <class T> std::ostream& operator<< (std::ostream& out, const VectorT<T>& v);
    template <class T> std::istream& operator>> (std::istream& in, VectorT<T>& v);
    template <class T> bool operator== (const VectorT<T>& v1, const VectorT<T>& v2);
    template <class T> bool operator== (const VectorT<T>& v1, const T& value);
    template <class T> bool operator!= (const VectorT<T>& v1, const VectorT<T>& v2);
    template <class T> bool operator!= (const VectorT<T>& v1, const T& value);
    template <class T> bool operator>  (const VectorT<T>& v1, const VectorT<T>& v2);
    template <class T> bool operator>= (const VectorT<T>& v1, const VectorT<T>& v2);
    template <class T> bool operator<  (const VectorT<T>& v1, const VectorT<T>& v2);
    template <class T> bool operator<= (const VectorT<T>& v1, const VectorT<T>& v2);
    template <class T> bool operator>  (const VectorT<T>& v1, const T& value);
    template <class T> bool operator>= (const VectorT<T>& v1, const T& value);
    template <class T> bool operator<  (const VectorT<T>& v1, const T& value);
    template <class T> bool operator<= (const VectorT<T>& v1, const T& value);
    template <class T> void copy       (const VectorT<T>& v1, VectorT<T>& v2);

////////////////////////////////////////////////////////////////////////////////

/// Definition of a class VectorT for numerical applications that stores the
/// elements on an array and provides some numerical functions.
/// Mathematically this VectorT is a vector in a Euclidean [n]-space, where n
/// is the dimension of the vector. All transformations assume a right handed
/// Cartesian frame of reference.
/// @author Andrea Lani
/// @author Tiago Quintino
template < typename T >
class Math_API VectorT : public Expr < VectorT<T>, T > {
public:

  typedef boost::detail::multi_array::sub_array<T,1> BoostRow;
  typedef boost::detail::multi_array::const_sub_array<T,1> ConstBoostRow;

  /// Constructor taking size, also works as empty constructor with default size to zero
  /// @param size number of elements in the vector
  explicit VectorT ( const Uint size = 0 );

  /// Constructor taking preallocated memory
  explicit VectorT ( const Uint size, T* mem );

  /// Constructor by size and with initializing value.
  /// @todo this can be called even if not wanted
  /// @param value initializing value
  /// @param size  number of elements in the vector
  explicit VectorT ( const T& value, const Uint size );

  /// Copy Constructor.
  /// @param orig source to copy from
  VectorT (const VectorT<T>& orig);

  /// Copy Constructor from an expression
  /// @param expr from which constructing the vector
  /// @param n    size of the vector to contruct
  template <class EXPR>
  VectorT(const Expr<EXPR,T>& expr) :
    Expr<VectorT<T>, T>(*this),
    m_owner (true),
    m_size  ( expr.size() ),
    m_data  (CFNULL)
  {
    alloc_mem();
    const size_t size = this->size();
    for (size_t i = 0; i < size; ++i) {
      m_data[i] = expr.at(i);
    }
  }

  /// Provide STL-compatible construction, useful for compatibility with boost::assign::list_of
  template< typename InputIterator >
  VectorT( InputIterator start, InputIterator end ) :
	  Expr<VectorT<T>,T>(*this),
      m_owner (true),
      m_size  ( end-start ),
      m_data  (CFNULL)
    {
      alloc_mem();
      size_t i = 0;
      for (InputIterator it = start; it != end; ++it) {
        m_data[i++] = *it;
      }
    }

  VectorT(const BoostRow& row) :
      Expr<VectorT<T>,T>(*this),
        m_owner (true),
        m_size  ( row.size() ),
        m_data  (CFNULL)
      {
        alloc_mem();
        size_t i = 0;
        for (typename BoostRow::const_iterator it = row.begin(); it != row.end(); ++it) {
          m_data[i++] = *it;
        }
      }

  VectorT(const ConstBoostRow& row) :
      Expr<VectorT<T>,T>(*this),
        m_owner (true),
        m_size  ( row.size() ),
        m_data  (CFNULL)
      {
        alloc_mem();
        size_t i = 0;
        for (typename ConstBoostRow::const_iterator it = row.begin(); it != row.end(); ++it) {
          m_data[i++] = *it;
        }
      }


  /// Destructor
  ~VectorT ();

  /// Overloading for operator= taking an expression as argument
#define VEC_EQ_OP(__op__) \
  template <class EXPR>          \
  const VectorT<T>& operator __op__ (const Expr<EXPR,T>& expr)  \
  {                \
    const size_t n = size();          \
    for (size_t i = 0; i < n; ++i) {        \
      m_data[i] __op__ expr.at(i);      \
    }            \
    return *this;        \
  }

VEC_EQ_OP(=)
VEC_EQ_OP(+=)
VEC_EQ_OP(-=)
VEC_EQ_OP(*=)
VEC_EQ_OP(/=)

#undef VEC_EQ_OP

  /// Overloading for operator= taking a constant value as argument
#define VEC_EQ_OP_CONST(__op__) \
  const VectorT<T>& operator __op__ (const T& value)  \
  {              \
    const size_t n = size();        \
    for (size_t i = 0; i < n; ++i) {      \
      m_data[i] __op__ value;        \
    }              \
    return *this;          \
  }

VEC_EQ_OP_CONST(=)
VEC_EQ_OP_CONST(+=)
VEC_EQ_OP_CONST(-=)
VEC_EQ_OP_CONST(*=)

#undef VEC_EQ_OP_CONST

  /// Overloading of "/="
  /// @param value of type T
  const VectorT<T>& operator/= (const T& value)
  {
    cf_assert( MathChecks::isNotZeroWithError(value, std::numeric_limits<T>::epsilon() ) );
    const size_t n = size();
    const T invalue = 1. / value;
    for ( size_t i = 0; i < n; ++i)
      m_data [i] *= invalue;
    return *this;
  }

  /// Overloading of the assignment operator "="
  /// @pre the assignee is supposed to have the same size
  ///      as this VectorT (as in std::valarray).
  /// @param other missing documentation
  /// @return missing documentation
  const VectorT<T>& operator= (const VectorT<T>& other)
  {
    cf_assert(&other != this);
    copy(other,*this);
    return *this;
  }

  /// Function returning a slice of this VectorT
  VectorSliceT<T> slice (const Uint& start)
  {
    return VectorSliceT<T>(&m_data[start]);
  }

  /// This allows to reset the inner pointer of the VectorT
  /// It must be used cautiously, only in case in which there is no ownership
  void resetPtr(T* ptr) {cf_assert(!m_owner);  m_data = ptr;}

  /// Accessor to the raw data
  /// This is to be used to pass the vector to C or Fortran functions
  /// Avoid using this function as much as possible
  T* getRawPtr () const { return m_data; }

  /// Accessor used by the expression template wrapper class
  T at (const Uint& iElem) const
  {
    cf_assert(iElem < size());
    return m_data[iElem];
  }

  /// Overloading of the "[]" operator for assignment (writing).
  /// @param iElem index
  T& operator[] (const Uint& iElem)
  {
    cf_assert(iElem < size());
    return m_data[iElem];
  }

  /// Overloading of the "[]" operator for assignment (reading only).
  /// @param iElem index
  T operator[] (const Uint& iElem) const
  {
    cf_assert(iElem < size());
    return m_data[iElem];
  }

  /// Gets the size (the number of elements) of the VectorT
  /// @return size of the VectorT
  Uint size() const { return m_size; }

  /// Calculates the sum of all values stored in VectorT
  /// @return T equal to sum of elements
  T sum () const;

  /// Calculates the norm L1 of the vector \f$\|V\|\f$
  /// @return T equal to the L1 norm of the vector.
  T norm1 () const;

  /// Calculates the norm L2 of the vector \f$\|V\|\f$
  /// In cartesian frame equals the size of the vector.
  /// @return T equal to the L2 norm of the vector.
  T norm2() const { return  std::sqrt(MathFunctions::innerProd(*this, *this)); }

  /// Calculates the norm $L_{\infty}$ of the vector \f$\|V\|\f$
  /// @return T equal to the norm of the vector.
  T normInf() const;

  /// Calculates the squared norm of the vector \f$\|V\|^2\f$
  /// Very usefull to compare norms of vectors because it saves the sqrt() call.
  /// @return T equal to sum of elements
  T sqrNorm() const { return MathFunctions::innerProd(*this, *this); }

  /// Normalizes this vector so that the norm is equal to one.
  /// \f$ v = v/\|v\|\f$
  void normalize() { *this /= std::sqrt(MathFunctions::innerProd(*this, *this));  }

  /// Clamps the values around zero to zero, by a neighbour fuzz value.
  /// @param fuzz threshold value to clamp the VectorT, if none is given,  machine-epsilon is used.
  /// @post all entries of this VectorT that have an absolute value that is
  /// less than fuzz have been set to zero.
  void clamp(const T& fuzz = std::numeric_limits<T>::epsilon());

  /// Projection of one vector onto another.
  /// @pre Objects must be of the same size.
  /// @param v1 1st VectorT
  /// @param v2 2nd VectorT
  /// @post this VectorT contains the projected vector of v2 onto v1.
  void proj(const VectorT<T>& v1, const VectorT<T>& v2)
  {
    // Mind that the precondition is not checked here, but in innerProd!!
    T scale = (MathFunctions::innerProd(v1, v2) / v1.sqrNorm());
    *this = v1 * scale;
  }

  /// Projection of this VectorT onto a given one.
  /// @pre this and the given object must be of the same size.
  /// @param v1 the other VectorT
  /// @post this VectorT contains the projected vector of itself onto v1.
  void proj(const VectorT<T>& v1) { proj(*this,v1); }

  /// Check if this VectorT and a given one are colinear.
  /// @param v1 the other VectorT
  /// @param fuzz minimal distance for comparison [optional]
  /// @return true if this and v1 are colinear.
  bool isColinear(const VectorT<T>& v1, const T& fuzz = std::numeric_limits<T>::epsilon()) const;

  /// Checks if this VectorT and a given one are orthogonal
  /// @param v1 the other VectorT
  /// @param fuzz minimal distance for comparison [optional]
  /// @return true if this and v1 are orthogonal.
  bool isOrthogonal(const VectorT<T>& v1,  const T& fuzz = std::numeric_limits<T>::epsilon()) const;

  /// Resize this VectorT. This is only done if the new size is different from the current one.
  /// @param size the new size of this VectorT
  /// @param value put the all values of the vector to this, default will be T()
  /// @post the size of this VectorT equals the given size.
  /// @post data stored is of unknown value.
  void resize (const Uint size, const T& value = T())
  {
    cf_always_assert_desc("Cannot use resize in a VectorT with preallocated memory", m_owner);
    if (size != m_size)
    {
      release_mem();
      // always true // m_owner = true;
      m_size = size;
      alloc_mem();
    }
    operator= (value);
  }

  /// Overloading of the stream operator "<<" for the output.
  /// No "\n"ine introduced.
  /// @param out missing documentation
  /// @param v missing documentation
  /// @return missing documentation
  friend std::ostream& operator<< <> (std::ostream& out, const VectorT<T>& v);

  /// Overloading of the stream operator ">>" for the input
  /// @param in missing documentation
  /// @param v missing documentation
  /// @return missing documentation
  friend std::istream& operator>> <> (std::istream& in, VectorT<T>& v);

  /// Overloading of the "==" operator.
  /// @param v1 missing documentation
  /// @param v2 missing documentation
  /// @return true if all elements are equal elementwise
  friend bool operator== <> (const VectorT<T>& v1, const VectorT<T>& v2);

  /// Overloading of the "==" operator.
  /// @param v given array
  /// @param value value for the comparison
  /// @return true if all elements are equal to value
  friend bool operator== <> (const VectorT<T>& v1, const T& value);

  /// Overloading of the "!=" operator.
  /// @param v1 missing documentation
  /// @param v2 missing documentation
  /// @return true if not all elements are equal elementwise
  friend bool operator!= <> (const VectorT<T>& v1, const VectorT<T>& v2);

  /// Overloading of the "!=" operator.
  /// @param v given array
  /// @param value value for the comparison
  /// @return true if there is at least one element not equal to value
  friend bool operator!= <> (const VectorT<T>& v1, const T& value);

  /// Overloading of the ">" operator.
  /// @param v1 missing documentation
  /// @param v2 missing documentation
  /// @return true if the norm of the first VectorT is bigger than  the norm of the second VectorT.
  friend bool operator> <> (const VectorT<T>& v1, const VectorT<T>& v2);

  /// Overloading of the ">=" operator.
  /// @param v1 missing documentation
  /// @param v2 missing documentation
  /// @return true if the norm of the first VectorT is bigger than or equal to the norm of the second VectorT.
  friend bool operator>= <> (const VectorT<T>& v1, const VectorT<T>& v2);

  /// Overloading of the "<=" operator.
  /// @param v1 missing documentation
  /// @param v2 missing documentation
  /// @return true if the norm of the first VectorT is smaller than the norm of the second VectorT.
  friend bool operator< <> (const VectorT<T>& v1, const VectorT<T>& v2);

  /// Overloading of the "<=" operator.
  /// @param v1 missing documentation
  /// @param v2 missing documentation
  /// @return true if the norm of the first VectorT is smaller than or equal to the norm of the second VectorT.
  friend bool operator<= <> (const VectorT<T>& v1, const VectorT<T>& v2);

  /// Overloading of the ">" operator.
  /// @param v1 missing documentation
  /// @param v2 missing documentation
  /// @return true if the norm of the first VectorT is bigger than the norm of the second VectorT.
  friend bool operator> <> (const VectorT<T>& v1, const T& value);

  /// Overloading of the ">=" operator.
  /// @param v1 missing documentation
  /// @param v2 missing documentation
  /// @return true if the norm of the first VectorT is bigger than or equal to the norm of the second VectorT.
  friend bool operator>= <> (const VectorT<T>& v1, const T& value);

  /// Overloading of the "<=" operator.
  /// @param v1 missing documentation
  /// @param v2 missing documentation
  /// @return true if the norm of the first VectorT is smaller than the norm of the second VectorT.
  friend bool operator< <> (const VectorT<T>& v1, const T& value);

  /// Overloading of the "<=" operator.
  /// @param v1 missing documentation
  /// @param v2 missing documentation
  /// @return true if the norm of the first VectorT is smaller than or equal to the norm of the second VectorT.
  friend bool operator<= <> (const VectorT<T>& v1, const T& value);

  /// Copy one VectorT into another one
  /// @pre v1.size() == v2.size()
  /// @param v1 source vector
  /// @param v2 destination vector
  friend void copy <> (const VectorT<T>& orig, VectorT<T>& dest);

  /// Compute the partial sum of the elements in the VectorT
  /// @param iStart start index
  /// @param iEnd   end index
  /// @return the partial sum from iStart to iEnd
  T partialSum(const Uint iStart, const Uint iEnd) const;

  /// Get the max in the vector
  T emax() const;

  /// Get the min in the vector
  T emin() const;

  ///  Put the absolute value (abs) of the given vector in this VectorT
  void abs (const VectorT<T>& v);

private: // helper functions

  /// Deletes the allocated memory in case of ownership
  void release_mem ();

  /// Allocates the memory for the size
  void alloc_mem () { cf_assert(m_owner && m_data == CFNULL); if ( m_size > 0 ) m_data = new T [m_size]; }

private: // data
  /// indicates ownership of the memory
  /// is false if memory was allocated outside
  bool m_owner;
  /// size of vector
  size_t m_size;
  /// Internal storage
  T* cf_restrict m_data;

}; // class VectorT

////////////////////////////////////////////////////////////////////////////////

template <class T>
VectorT<T>::VectorT ( const Uint size ) : Expr<VectorT<T>,T>(*this),
  m_owner (true),
  m_size  (size),
  m_data  (CFNULL)
{
  alloc_mem();
  operator= (T());
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
VectorT<T>::VectorT(const Uint size, T* ptr) :
  Expr<VectorT<T>,T>(*this),
  m_owner (false),
  m_size  (size),
  m_data  (ptr)
{
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
VectorT<T>::VectorT ( const T& value, const Uint size ) :
  Expr<VectorT<T>,T>(*this),
  m_owner (true),
  m_size  (size),
  m_data  (CFNULL)
{
  alloc_mem();
  operator= (value);
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
VectorT<T>::~VectorT()
{
  release_mem();
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
void VectorT<T>::release_mem ()
{
  if (m_owner && m_data != CFNULL)
  {
    delete [] m_data;
    m_data = CFNULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
VectorT<T>::VectorT(const VectorT<T>& orig) :
  Expr<VectorT<T>,T>(*this),
  m_owner (true),
  m_size  (orig.m_size),
  m_data  (CFNULL)
{
  alloc_mem();
  //    for (size_t i = 0; i < size; ++i) m_data [i] = orig.m_data [i];
  //    memcpy( m_data, orig.m_data, ( (orig.m_data + m_size ) - orig.m_data ) * sizeof(T) );
  copy(orig,*this);
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline void VectorT<T>::clamp(const T& fuzz)
{
  const size_t size = this->size();
  for (size_t i = 0; i < size; ++i) {
    if (std::abs(m_data[i]) < fuzz) {
      m_data[i] = T();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline T VectorT<T>::emax () const
{
  const size_t size = this->size();
  cf_assert (size > 0);
  T rmax = m_data[0];
  for (size_t i = 1; i < size; ++i)
    rmax = std::max(m_data[i],rmax);
  return rmax;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline T VectorT<T>::emin () const
{
  const size_t size = this->size();
  cf_assert (size > 0);
  T rmin = m_data[0];
  for (size_t i = 1; i < size; ++i)
    rmin = std::min(m_data[i],rmin);
  return rmin;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline T VectorT<T>::sum () const
{
  T ret_sum = (T) 0;
  const size_t size = this->size();
  for (size_t i = 0; i < size; ++i)
    ret_sum += m_data[i];
  return ret_sum;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline T VectorT<T>::norm1 () const
{
  T norm = (T) 0;
  const size_t size = this->size();
  for (size_t i = 0; i < size; ++i)
    norm += std::abs( m_data[i] );
  return norm;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline T VectorT<T>::normInf () const
{
  const size_t size = this->size();
  cf_assert (size > 0);
  T norm = std::abs(m_data[0]);
  for (size_t i = 1; i < size; ++i)
    norm = std::max ( norm, std::abs(m_data[i]) );
  return norm;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool VectorT<T>::isColinear(const VectorT<T>& v1, const T& fuzz) const
{
  const size_t size = this->size();
  cf_assert (size > 0);
  cf_assert(v1.size() == size);
  T ratio1 = v1.m_data[0]/m_data[0];
  T ratio2;
  for (size_t i = 1; i < size; ++i) {
    ratio2 = v1.m_data[i]/m_data[i];
    if (std::abs(ratio1 - ratio2) > fuzz) {
      return false;
    }
    ratio1 = ratio2;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline bool VectorT<T>::isOrthogonal(const VectorT<T>& v1, const T& fuzz) const
{
  return (std::abs(MathFunctions::innerProd(*this, v1)) < fuzz);
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline T VectorT<T>::partialSum(const Uint iStart, const Uint iEnd) const
{
  T result = T();
  for (size_t i = iStart; i < iEnd; ++i) {
    result += m_data[i];
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
std::ostream& operator<< (std::ostream& out, const VectorT<T>& v)
{
  const size_t size = v.size();
  for (size_t i = 0; i < size; ++i)
    out << v.m_data[i] << " " ;
  return out;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
std::istream& operator>> (std::istream& in, VectorT<T>& v)
{
  const size_t size = v.size();
  for (size_t i = 0; i < size; ++i)
    in >> v.m_data[i];
  return in;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator== (const VectorT<T>& v1, const VectorT<T>& v2)
{
  cf_assert(v1.size() == v2.size());
  const size_t size = v1.size();
  for (size_t i = 0; i < size; ++i) {
    if (v1.m_data[i] != v2.m_data[i]) {
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator== (const VectorT<T>& v, const T& value)
{
  const size_t size = v.size();
  for (size_t i = 0; i < size; ++i) {
    if (v[i] != value)  return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator!= (const VectorT<T>& v1, const VectorT<T>& v2)
{
  return !(v1 == v2);
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator!= (const VectorT<T>& v, const T& value)
{
  return !(v == value);
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator> (const VectorT<T>& v1, const VectorT<T>& v2)
{
  return v1.sqrNorm() > v2.sqrNorm();
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator>= (const VectorT<T>& v1, const VectorT<T>& v2)
{
  return !(v1.sqrNorm() < v2.sqrNorm());
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator< (const VectorT<T>& v1, const VectorT<T>& v2)
{
  return v1.sqrNorm() < v2.sqrNorm();
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator<= (const VectorT<T>& v1, const VectorT<T>& v2)
{
  return !(v1.sqrNorm() > v2.sqrNorm());
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator> (const VectorT<T>& v1, const T& value)
{
  const size_t size = v1.size();
  for (size_t i = 0; i < size; ++i) {
    if (v1[i] <= value) return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator>= (const VectorT<T>& v1, const T& value)
{
  const size_t size = v1.size();
  for (size_t i = 0; i < size; ++i) {
    if (v1[i] < value) return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator< (const VectorT<T>& v1, const T& value)
{
  const size_t size = v1.size();
  for (size_t i = 0; i < size; ++i) {
    if (v1[i] >= value) return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
bool operator<= (const VectorT<T>& v1, const T& value)
{
  const size_t size = v1.size();
  for (size_t i = 0; i < size; ++i) {
    if (v1[i] > value) return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
void copy (const VectorT<T>& orig, VectorT<T>& dest)
{
  cf_assert(orig.size() == dest.size());
  const size_t size = orig.size();
  for (size_t i = 0; i < size; ++i)
    dest.m_data[i] = orig.m_data[i];
}

////////////////////////////////////////////////////////////////////////////////

template <class T>
inline void VectorT<T>::abs(const VectorT<T>& v)
{
  cf_assert (m_size == v.size());
  const size_t size = this->size();
  for (size_t i = 0; i < size; ++i)
    m_data[i] = std::abs(v[i]);
}

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

}   // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_VectorT_hpp
