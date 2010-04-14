#ifndef COOLFluiD_MathTools_LExprOp_hh
#define COOLFluiD_MathTools_LExprOp_hh

//////////////////////////////////////////////////////////////////////////////

#include "Cmp.hh"

//////////////////////////////////////////////////////////////////////////////

namespace COOLFluiD {
  
  namespace MathTools {
    
    template <class T, class TAG, CFuint N> class MatrixLET;
    template <class T, class TAG, CFuint N> class VectorLET;
    
//////////////////////////////////////////////////////////////////////////////
    
/**
 * Definition of an empty tag class
 *
 * @author Andrea Lani
 *
 */
template <class T1, CFuint T2> struct Tag {};
  
/**
 * Definition of a lightweight wrapper base class for size-deducing fast 
 * expression templates. It simply holds the knowledge of the expression 
 * size. 
 *
 * @author Andrea Lani
 *
 */
template <class DATA, class T, CFuint NUM>
class LExpr {
public:
  enum {SIZE=NUM};
};

/**
 * Definition of a wrapper class for a scalar value
 *
 * @author Andrea Lani
 *
 */
template <class T, class TAG>
class Scalar : public LExpr<Scalar<T,TAG>,T,0> {
public:
  /**
   * Constructor accepting a value as input
   */
  explicit Scalar(const T& value) {_value = value;}
  
  /**
   * Accessor used by the expression template engine
   */
  static inline T at(CFuint i)  {return _value;}
  
private:
  /// data
  static T _value;
};

/**
 * Instantiation of static data
 */
template <class T, class TAG>
T Scalar<T,TAG>::_value = T();

/**
 * Definition of an expression template class for basic binary operations.
 * A macro is used to save tedious code duplication.
 * The expression template accepts three parameters:
 * 1. first operand expression type
 * 2. second operand expression type
 * 3. built-in type of both the expressions (double, float, etc.).
 *
 * The constructor accepts as arguments two references to expression 
 * template objects (corrresponding to the two operands of the expression).
 *
 * @author Andrea Lani
 *
 */   
#define LET_BINARY_OP(__OpName__,__op__)		\
  template <class V1, class V2, class T>				\
  class __OpName__ : public LExpr<__OpName__<V1,V2,T>, T, NMAX(V1,V2)> { \
  public:								\
    __OpName__ (const LExpr<V1,T,V1::SIZE>& v1, const LExpr<V2,T,V2::SIZE>& v2) : \
      LExpr< __OpName__<V1,V2,T>,T,NMAX(V1,V2)>() {}			\
      									\
      static inline T at(CFuint i)						\
      {									\
	return V1::at(i) __op__ V2::at(i);				\
      }									\
  };
						
LET_BINARY_OP(LETNAdd,+)
LET_BINARY_OP(LETNSub,-)
LET_BINARY_OP(LETNMult,*)
LET_BINARY_OP(LETNDiv,/)

#undef LET_BINARY_OP

/**
 * Expression template class for multiplication between a MatrixLET 
 * and another type (e.g. VectorLET or expressions involving VectorLET).
 * The expression template accepts 4 parameters:
 * 1. first operand in the expression
 * 2. built-in type of both the expressions (double, float, etc.).
 * 3. tag class
 * 4. the expression size.
 * The constructor accepts as arguments two references, one to a 
 * MatrixLET expression template object, the other to an expression template 
 * object.
 *
 * @author Andrea Lani
 *
 */ 
template <class V2, class T, class TAG, CFuint N>
class MVMult : 
  public LExpr<MVMult<V2,T,TAG, N>, T, NNMAX(N,V2)> {
    
  public:					
    /**
     * Default Constructor.
     */
    MVMult(const MatrixLET<T,TAG,N>& v1, 
	   const LExpr<V2, T, V2::SIZE>& v2) :
      LExpr<MVMult<V2,T,TAG, N>, T, NNMAX(N,V2)>()
      {
      }
   
   /**
    * Accessor used by the expression template engine
    */
    static inline T at(CFuint i) {
      T res = T();
      const CFuint n = MatrixLET<T,TAG,N>::nbCols();
      for (CFuint j = 0, k = i*n; j < n; ++j, ++k) {
	res += MatrixLET<T,TAG,N>::at(k)*V2::at(j);
      }
      return res;
    }						
  };

/**
 * Inline function (the keyword must be put otherwise the compiler will not inline it,
 * as shown by the profiler) providing the overloading of the basic binary operators
 * between two expression template objects.
 *
 * @param v1 expression template object (first operand)
 * @param v2 expression template object (second operand)
 */
#define OVERLOAD_BINARY_LET_OP(__OpName__,__op__) \
  template <class V1, class V2, class T>			\
  inline __OpName__<V1, V2, T> operator __op__ (const LExpr<V1,T,V1::SIZE>& v1, const LExpr<V2,T,V2::SIZE>& v2) \
{\
  return __OpName__<V1,V2,T>(v1,v2);		\
}

OVERLOAD_BINARY_LET_OP(LETNAdd,+)
OVERLOAD_BINARY_LET_OP(LETNSub,-)
OVERLOAD_BINARY_LET_OP(LETNMult,*)
OVERLOAD_BINARY_LET_OP(LETNDiv,/)

#undef OVERLOAD_BINARY_LET_OP
  
/**
 * Inline function (the keyword must be put otherwise the compiler will not inline it,
 * as shown by the profiler) providing the overloading of the basic binary operators
 * between one MatrixLET expression template and another expression template object.
 *
 * @param v1  expression template object (first operand)
 * @param v2 expression template object (second operand)
 */
template <class V2, class T, class TAG, CFuint N>
inline MVMult<V2, T, TAG, N> operator* (const MatrixLET<T,TAG,N>& v1,
					const LExpr<V2,T,V2::SIZE>& v2)
{
  return MVMult<V2, T, TAG, N>(v1,v2);
}
    
/**
 * Expression template class for multiplication between a MatrixLET
 * and a LETNMult applied to a VectorLET and a MatrixLET.
 * It allows to tackle operations such as M3 = M1*(v*M2).
 * The expression template accepts 7 parameters:
 * 1. built-in type of both the expressions (double, float, etc.).
 * 2. tag class for first MatrixLET
 * 3. tag class for VectorLET
 * 4. tag class for second MatrixLET
 * 5. expression size for first MatrixLET
 * 6. expression size for VectorLET
 * 7. expression size for second MatrixLET
 *
 * The constructor accepts as arguments two references, one to a
 * MatrixLET expression template object, the other to the LETNMult 
 * expression template object, specialized forthe case VectorLET*MatrixLET.
 *
 * @author Andrea Lani
 *
 */
template <class T, class TAG1, class TAG2, class TAG3, 
	  CFuint N1, CFuint N2, CFuint N3>
class MVMMult : public LExpr<MVMMult<T,TAG1,TAG2,TAG3,N1,N2,N3>,T,
			     CMP<N3,CMP<N1,N2>::MAX>::MAX> {
public:					
  MVMMult(const MatrixLET<T,TAG1,N1>& v1,
	  const LETNMult<VectorLET<T,TAG2,N2>,MatrixLET<T,TAG3,N3>,T>& v2) :
    LExpr<MVMMult<T,TAG1,TAG2,TAG3,N1,N2,N3>,T,CMP<N3,CMP<N1,N2>::MAX>::MAX>()
  {
  }
  
  static inline T at(CFuint i) {
    typedef MatrixLET<T,TAG1,N1> M1;
    typedef MatrixLET<T,TAG3,N3> M3;
    typedef VectorLET<T,TAG2,N2> V1;
    
    T res = T();
    const CFuint jc = i%M3::nbCols();
    for (CFuint j = 0, k = (i/M3::nbCols())*M1::nbCols(); 
	 j < M1::nbCols(); ++j, ++k) {
      res += M1::at(k)*M3::at(jc + M3::nbCols()*j)*V1::at(j);
    }
    return res;
  }
};
  
/**
 * Inline function (the keyword must be put otherwise the compiler will not inline it,
 * as shown by the profiler) providing the overloading of the product
 * between a MatrixLET object and a LETNMult expression template applied to a VectorLET 
 * and a MatrixLET, in order to tackle expressions such as M3 = M1*(v*M2).
 *
 * @param v1  expression template object (first operand)
 * @param v2  expression template object (second operand)
 */
template <class T, class TAG1, class TAG2, class TAG3, 
	  CFuint N1, CFuint N2, CFuint N3>
inline MVMMult<T, TAG1, TAG2, TAG3, N1,N2,N3> operator* 
(const MatrixLET<T,TAG1, N1>& v1,
 const LETNMult<VectorLET<T,TAG2,N2>,MatrixLET<T,TAG3,N3>,T>& v2)
{
  return MVMMult<T, TAG1, TAG2, TAG3, N1,N2,N3>(v1,v2);
}

/**
 * Expression template class for multiplication between two MatrixLET's.
 * The expression template accepts 5 parameters:
 * 1. built-in type of both the expressions (double, float, etc.).
 * 2. tag class for first MatrixLET
 * 3. tag class for second MatrixLET
 * 4. expression size for first MatrixLET
 * 5. expression size for second MatrixLET
 *
 * The constructor accepts as arguments two references to MatrixLET objects.
 *
 * @author Andrea Lani
 *
 */
template <class T, class TAG1, class TAG2, CFuint N1, CFuint N2>
class MMMult : public LExpr<MMMult<T,TAG1,TAG2,N1,N2>, T, CMP<N1,N2>::MAX> {
public:					
  MMMult(const MatrixLET<T,TAG1,N1>& v1, 
	 const MatrixLET<T,TAG2,N2>& v2) :
    LExpr<MMMult<T,TAG1,TAG2,N1,N2>, T,CMP<N1,N2>::MAX>() {}
  
  static inline T at(CFuint i) {
    typedef MatrixLET<T,TAG1,N1> M1;
    typedef MatrixLET<T,TAG2,N2> M2;
    
    T res = T();
    const CFuint jc = i%M2::nbCols();
    for (CFuint j = 0, k = (i/M2::nbCols())*M1::nbCols(); 
	 j < M1::nbCols(); ++j, ++k) {
      res += M1::at(k)*M2::at(jc + M2::nbCols()*j);
    }
    return res;
  }						
};

/**
 * Inline function (the keyword must be put otherwise the compiler will not inline it,
 * as shown by the profiler) providing the overloading of the product
 * between two MatrixLET objects, in order to tackle expressions such as A = B*C.
 *
 * @param v1  expression template object (first operand)
 * @param v2  expression template object (second operand)
 */
template <class T, class TAG1, class TAG2, CFuint N1, CFuint N2>
inline MMMult<T,TAG1,TAG2,N1,N2> operator* 
(const MatrixLET<T,TAG1,N1>& v1, const MatrixLET<T,TAG2,N2>& v2)
{
  return MMMult<T,TAG1,TAG2,N1,N2>(v1,v2);
}


//////////////////////////////////////////////////////////////////////////////

  } // namespace MathTools

}   // namespace COOLFluiD

//////////////////////////////////////////////////////////////////////////////

#endif // COOLFluiD_MathTools_LExprOp_hh
