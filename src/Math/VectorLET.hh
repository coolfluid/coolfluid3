#ifndef COOLFluiD_MathTools_VectorLET_hh
#define COOLFluiD_MathTools_VectorLET_hh

//////////////////////////////////////////////////////////////////////////////



#include "CFVector.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {

  namespace MathTools {
    
    template <class T, class TAG, CFuint N> 
    class VectorLET;
    
    template <class T, class TAG, CFuint N> 
    std::ostream& operator<< (std::ostream& out, const VectorLET<T,TAG,N>& v);
    
    template <class T, class TAG, CFuint N>
    std::istream& operator>> (std::istream& in, VectorLET<T,TAG,N>& v);
    
    template <class T, class TAG> 
    std::ostream& operator<< (std::ostream& out, const VectorLET<T,TAG,0>& v);
    
    template <class T, class TAG>
    std::istream& operator>> (std::istream& in, VectorLET<T,TAG,0>& v);
    
//////////////////////////////////////////////////////////////////////////////

/**
 * Definition of a high-performance vector class for numerical applications 
 * that uses an improved version of the fast expression template concept.
 * This techique allows to retain a user-friendly Matlab-like syntax at the cost
 * of two extra template parameters: 
 * 1- a Tag class which ensures unicity of the vector instantiation within 
 *    the context of a class) 
 * 2- an integer that distinguish a C style allocated storage with fixed size from 
 *    a dynamical allocated one.
 *
 * @author Andrea Lani
 *
 */
template <class T, class TAG, CFuint N = 0>
class VectorLET : public LExpr<VectorLET<T,TAG,N>,T,N> {
public:
  
  /**
   * Default Constructor.
   */
  VectorLET();
  
  /**
   * Destructor.
   */
  ~VectorLET();
  
  /**
   * Overloading of the stream operator "<<" for the output
   * No "\n"ine introduced.
   *
   * @param out missing documentation
   * @param v missing documentation
   * @return missing documentation
   */
  friend std::ostream& operator<< LTGT (std::ostream& out, 
					const VectorLET<T,TAG,N>& v);
  
  /**
   * Overloading of the stream operator ">>" for the input
   *
   * @param in missing documentation
   * @param v missing documentation
   * @return missing documentation
   */
  friend std::istream& operator>> LTGT (std::istream& in, 
					VectorLET<T,TAG,N>& v);
  
  /**
   * Accessor used by the expression template engine
   */
  static inline T at (CFuint i) {return _v[i];}
  
  /**
   * Overloading of the "[]" operator for assignment (writing).
   * @param i index
   */
  T& operator[] (CFuint i) {return _v[i];}
  
  /**
   * Overloading of the "[]" operator for assignment (reading).
   * @param i index
   */
  const T& operator[] (CFuint i) const {return _v[i];} 
  
  /**
   * Gets the size (the number of elements) of the VectorLET
   * @return size of the VectorLET
   */
  CFuint size() const {return N;}
  
  /**
   * Overloading for operator= taking an expression as argument
   */  
#define VECLET_EQ_OP(__op__)						\
  template <class EXPR>							\
  const VectorLET& operator __op__ (const LExpr<EXPR,T,EXPR::SIZE>& expr) \
{									\
  for (CFuint i = 0; i < N; ++i) {				\
    _v[i] __op__ EXPR::at(i);}					\
  return *this;								\
}

  VECLET_EQ_OP(=)
  VECLET_EQ_OP(+=)
  VECLET_EQ_OP(-=)
  VECLET_EQ_OP(*=)
  VECLET_EQ_OP(/=)

#undef VECLET_EQ_OP
    
private:
  
  /// static array of data
  static T _v[N];
};

/**
 * Partial specialization of the VectorLET class in the case of an 
 * underlying dynamically allocated array 
 *
 * @author Andrea Lani
 *
 */
template <class T, class TAG>
class VectorLET<T,TAG,0> : public LExpr<VectorLET<T,TAG,0>,T,0> {
public:
  /**
   * Constructor  accepting the size of the dynamically allocated
   * raw storage as input
   */
  explicit VectorLET(CFuint ns);
  
  /**
   * Destructor
   */
  ~VectorLET();
  
    /**
     * Overloading of the stream operator "<<" for the output
   * No "\n"ine introduced.
   *
   * @param out missing documentation
   * @param v missing documentation
   * @return missing documentation
   */
  friend std::ostream& operator<< LTGT (std::ostream& out, 
					const VectorLET<T,TAG,0>& v);
  
  /**
   * Overloading of the stream operator ">>" for the input
   *
   * @param in missing documentation
   * @param v missing documentation
   * @return missing documentation
   */
  friend std::istream& operator>> LTGT (std::istream& in, 
					VectorLET<T,TAG,0>& v);
  
  /**
   * Accessor used by the expression template engine
   */
  static inline T at (CFuint i) 
  {
    return _v[i];
  }
  
  /**
   * Overloading of the "[]" operator for assignment (writing).
   * @param i index
   */
  T& operator[] (CFuint i) 
  {
    cf_assert(i < size());
    return _v[i];
  }
  
  /**
   * Overloading of the "[]" operator for assignment (reading).
   * @param i index
   */
  const T& operator[] (CFuint i) const
  { 
    cf_assert(i < size());
    return _v[i];
  } 
  
  /**
   * Gets the size (the number of elements) of the VectorLET
   * @return size of the VectorLET
   */
  CFuint size() const {return _size;}
  
  /**
   * Overloading of operator= in order to convert a given vector of type OTHER 
   * to a VectorLET
   */
  template <class OTHER>
  const VectorLET<T,TAG,0>& operator=(OTHER& array)
  {
    _v = &array[0];
    _size = array.size();
    return *this;
  }

  /**
   * Overloading of operator= in order to convert a given vector of type OTHER 
   * to a VectorLET
   */
  template <class OTHER>
  void setPtr(OTHER& array)
  {
    _v = &array[0];
  }
  
  /**
   * This function must be called ater having used the operator=(CFVector) 
   * in order to reset the pointer to default behaviour
   */
  void release() 
  {
    _v = CFNULL; 
    _size = 0;
  }
  
  /**
   * Overloading for operator= taking an expression as argument
   */ 
#define VECLET_EQ_OP(__op__)						\
template <class EXPR>							\
const VectorLET& operator __op__ (const LExpr<EXPR,T,EXPR::SIZE>& expr) \
{									\
  const CFuint nmax = CMP<0,EXPR::SIZE>::MAX;			\
  for (CFuint i = 0; i < GETSIZE(nmax); ++i) {				\
    _v[i] __op__ EXPR::at(i);}					\
  return *this;								\
}

  VECLET_EQ_OP(=)
  VECLET_EQ_OP(+=)
  VECLET_EQ_OP(-=)
  VECLET_EQ_OP(*=)
  VECLET_EQ_OP(/=)

#undef VECLET_EQ_OP
    
private:

  /// array size
  CFuint    _size;
  
  /// static pointer to raw data
  static T* _v;
};

//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

}   // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#include "VectorLET.ci"

//////////////////////////////////////////////////////////////////////////////

#endif
