#ifndef CF_Math_NExprOp_hh
#define CF_Math_NExprOp_hh

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

    template <class T, int N> class CFNMatrix;
    template <class T, int N> class CFNVector;
    template <class T, int N> class CFSliceNVector;
    template <class T, int N> class CFSliceNMatrix;

////////////////////////////////////////////////////////////////////////////////

* Definition of a wrapper base expression class NExpr, the closure objects
* (derived from NExpr using the CRTP technique), some partial or
* explicit specializations of the closure objects to treat special cases
* and definition of the overloading functions accepting expressions or constants
* as arguments.
* @author Andrea Lani
template <class DATA, class T, int N>
class NExpr {
public:

  NExpr(const DATA& data) : _data(data) {}

  T at (Uint i) const {return _data.at(i);}

  Uint size() const {return _data.size();}

  const DATA& getData() const {return _data;}

private:
  const DATA& _data;
};

* Definition of an expression template class for basic binary operations.
* A macro is used to save tedious code duplication.
* The expression template accepts three parameters:
* 1. first operand expression type
* 2. second operand expression type
* 3. built-in type of both the expressions (double, float, etc.).
* The constructor accepts as arguments two references to expression
* template objects (corrresponding to the two operands of the expression).
* @author Andrea Lani
#define NEXPR_BINARY_OP(__OpName__,__op__)    \
template <class V1, class V2, class T, int N>      \
class __OpName__ : public NExpr< __OpName__<V1,V2,T,N>, T,N> {  \
public:            \
  __OpName__(const NExpr<V1,T,N>& v1, const NExpr<V2,T,N>& v2) :  \
    NExpr< __OpName__<V1,V2,T,N>, T,N>(*this),        \
    ex1(v1), ex2(v2) {}             \
                     \
  T at (Uint i) const             \
  {              \
    return ex1.at(i) __op__ ex2.at(i);      \
  }              \
                \
  Uint size() const {return ex1.size();}    \
                \
  const NExpr<V1,T,N>& ex1;        \
  const NExpr<V2,T,N>& ex2;        \
};

NEXPR_BINARY_OP(Add,+)
NEXPR_BINARY_OP(Sub,-)
NEXPR_BINARY_OP(Mult,*)
NEXPR_BINARY_OP(Div,/)

#undef NEXPR_BINARY_OP

* Partial specialization of the previous expression template class
* for basic binary operations between a constant and an expression.
* A macro is used to save tedious code duplication.
* The expression template accepts two parameters:
* 2. second operand expression type
* 3. built-in type of both the expressions (double, float, etc.).
* The constructor accepts as arguments two references, one to constant,
* the other to an expression template object.
*
* @author Andrea Lani
*
#define NEXPR_BINARY_OP_CONST1(__OpName__,__op__)      \
template <class V2, class T, int N>            \
class __OpName__<T,V2,T,N> : public NExpr< __OpName__<T,V2,T,N>, T,N> {  \
public:            \
  __OpName__(const T& v1, const NExpr<V2,T,N>& v2) :  \
    NExpr< __OpName__<T,V2,T,N>,T,N>(*this),      \
    ex1(v1), ex2(v2) {}             \
                     \
  T at(Uint i) const             \
  {              \
    return ex1 __op__ ex2.at(i);      \
  }              \
                \
  Uint size() const {return ex2.size();}    \
                \
private:            \
const T& ex1;            \
  const NExpr<V2,T,N>& ex2;        \
};

NEXPR_BINARY_OP_CONST1(Add,+)
NEXPR_BINARY_OP_CONST1(Sub,-)
NEXPR_BINARY_OP_CONST1(Mult,*)
NEXPR_BINARY_OP_CONST1(Div,/)

#undef NEXPR_BINARY_OP_CONST1

* Partial specialization of the previous expression template class
* for basic binary operations between an expression and a constant.
* A macro is used to save tedious code duplication.
* The expression template accepts two parameters:
* 2. first operand expression type
* 3. built-in type of both the expressions (double, float, etc.).
*
* The constructor accepts as arguments two references, one to an
* expression template object, the other to a constant.
*
* @author Andrea Lani
*
#define NEXPR_BINARY_OP_CONST2(__OpName__,__op__)      \
template <class V1, class T, int N>        \
class __OpName__<V1,T,T,N> : public NExpr< __OpName__<V1,T,T,N>, T,N> {  \
public:            \
  __OpName__(const NExpr<V1,T,N>& v1, const T& v2) :  \
    NExpr< __OpName__<V1,T,T,N>,T,N>(*this),      \
    ex1(v1), ex2(v2) {}             \
                     \
  T at(Uint i) const             \
  {              \
    return ex1.at(i) __op__ ex2;      \
  }              \
                \
  Uint size() const {return ex1.size();}    \
                \
private:          \
  const NExpr<V1,T,N>& ex1;      \
  const T&          ex2;      \
};

NEXPR_BINARY_OP_CONST2(Add,+)
NEXPR_BINARY_OP_CONST2(Sub,-)
NEXPR_BINARY_OP_CONST2(Mult,*)
NEXPR_BINARY_OP_CONST2(Div,/)

#undef NEXPR_BINARY_OP_CONST2

* Partial specialization of the previous expression template class
* for multiplication between a CFNMatrix and another type
* (e.g. CFNVector or expressions involving CFNVectors).
* The expression template accepts two parameters:
* 2. second operand expression type
* 3. built-in type of both the expressions (double, float, etc.).
* The constructor accepts as arguments two references, one to a
* CFNMatrix expression template object, the other to an expression template
* object.
*
* @author Andrea Lani
*
template <class V2, class T, int N>
class Mult<CFNMatrix<T,N>,V2,T,N> : public NExpr< Mult<CFNMatrix<T,N>,V2,T,N>,T,N> {
public:
  Mult(const NExpr<CFNMatrix<T,N>,T,N>& v1, const NExpr<V2,T,N>& v2) :
    NExpr<Mult<CFNMatrix<T,N>,V2,T,N>, T,N>(*this),
    ex1(v1), ex2(v2) {}

  T at(Uint i) const
  {
    T res = T();
    const Uint n = ex1.getData().nbCols();
    for (Uint j = 0, k = i*n; j < n; ++j, ++k) {
      res += ex1.at(k)*ex2.at(j);
    }
    return res;
  }

  Uint size() const {return ex2.size();}

private:
  const NExpr<CFNMatrix<T,N>, T,N>& ex1;
  const NExpr<V2,T,N>&         ex2;
};

* Partial specialization of the previous expression template class
* for multiplication between two CFNMatrix's.
* The expression template accepts one parameter, namely the
* built-in type of both the expressions (double, float, etc.).
*
* The constructor accepts as arguments two references to
* CFNMatrix expression template objects.
*
* @author Andrea Lani
*
template <class T, int N>
class Mult<CFNMatrix<T,N>,CFNMatrix<T,N>,T,N> : public NExpr< Mult<CFNMatrix<T,N>,CFNMatrix<T,N>,T,N>,T,N> {
  friend class CFNMatrix<T,N>;
public:
  Mult(const NExpr<CFNMatrix<T,N>,T,N>& v1, const NExpr<CFNMatrix<T,N>,T,N>& v2) :
    NExpr<Mult<CFNMatrix<T,N>,CFNMatrix<T,N>,T,N>, T,N>(*this),
    ex1(v1), ex2(v2) {}

  T at(Uint i) const
  {
    T res = T();
    const Uint jc = i%ex2.getData().nbCols();
    for (Uint j = 0, k = (i/ex2.getData().nbCols())*ex1.getData().nbCols();
  j < ex1.getData().nbCols(); ++j, ++k) {
      res += ex1.at(k)*ex2.at(jc + ex2.getData().nbCols()*j);
    }
    return res;
  }

  Uint size() const {return ex1.getData().nbRows()*ex2.getData().nbCols();}

private:
  const NExpr<CFNMatrix<T,N>, T,N>& ex1;
  const NExpr<CFNMatrix<T,N>, T,N>& ex2;
};

* Partial specialization of the previous expression template class
* for multiplication of the type CFNMatrix*CFNVector*CFNMatrix,
* being CFNVector a diagonal matrix.
* The expression template accepts one parameter, namely the
* built-in type of both the expressions (double, float, etc.).
*
* The constructor accepts as arguments two references, one to
* a CFNMatrix and the other one to an expression template object
* corresponding to the CFNVector-CFNMatrix product.
*
* @author Andrea Lani
*
template <class T, int N>
class Mult<CFNMatrix<T,N>,Mult<CFNVector<T,N>,CFNMatrix<T,N>,T,N>,T,N> :
  public NExpr<Mult<CFNMatrix<T,N>,Mult<CFNVector<T,N>,CFNMatrix<T,N>,T,N>,T,N>,T,N> {

  friend class CFNMatrix<T,N>;
public:
  Mult(const NExpr<CFNMatrix<T,N>,T,N>& v1,
      const NExpr<Mult<CFNVector<T,N>,CFNMatrix<T,N>,T,N>,T,N>& v2) :
    NExpr<Mult<CFNMatrix<T,N>,Mult<CFNVector<T,N>,CFNMatrix<T,N>,T,N>,T,N>,T,N>(*this),
    ex1(v1), ex2(v2) {}

  T at(Uint i) const
  {
    T res = T();
    const Uint jc = i%ex2.getData().ex2.getData().nbCols();
    for (Uint j = 0, k = (i/ex2.getData().ex2.getData().nbCols())*ex1.getData().nbCols();
  j < ex1.getData().nbCols(); ++j, ++k) {
      res += ex1.at(k)*ex2.getData().ex2.at(jc + ex2.getData().ex2.getData().nbCols()*j)*ex2.getData().ex1.at(j);
    }
    return res;
  }

  Uint size() const
  {
    return ex1.getData().nbRows()*
      ex2.getData().ex2.getData().nbCols();
  }

private:
  const NExpr<CFNMatrix<T,N>, T,N>& ex1;
  const NExpr<Mult<CFNVector<T,N>,CFNMatrix<T,N>,T,N>,T,N>& ex2;
};

* Partial specialization of the previous expression template class
* for multiplication between a CFSliceNMatrix and another type
* (e.g. CFNVector or expressions involving CFNVectors).
* The expression template accepts two parameters:
* 2. second operand expression type
* 3. built-in type of both the expressions (double, float, etc.).
* The constructor accepts as arguments two references, one to a
* CFSliceNMatrix expression template object, the other to an expression template
* object.
*
* @author Andrea Lani
*
template <class V2, class T, int N>
class Mult<CFSliceNMatrix<T,N>,V2,T,N> : public NExpr< Mult<CFSliceNMatrix<T,N>,V2,T,N>,T,N> {
public:
  Mult(const NExpr<CFSliceNMatrix<T,N>,T,N>& v1, const NExpr<V2,T,N>& v2) :
    NExpr<Mult<CFSliceNMatrix<T,N>,V2,T,N>, T,N>(*this),
    ex1(v1), ex2(v2) {}

  T at(Uint i) const
  {
    T res = T();
    const Uint n = ex1.getData().nbCols();
    for (Uint j = 0, k = i*n; j < n; ++j, ++k) {
      res += ex1.at(k)*ex2.at(j);
    }
    return res;
  }

  Uint size() const {return ex2.size();}

private:
  const NExpr<CFSliceNMatrix<T,N>, T,N>& ex1;
  const NExpr<V2,T,N>&         ex2;
};

* Partial specialization of the previous expression template class
* for multiplication between two CFSliceNMatrix's.
* The expression template accepts one parameter, namely the
* built-in type of both the expressions (double, float, etc.).
*
* The constructor accepts as arguments two references to
* CFSliceNMatrix expression template objects.
*
* @author Andrea Lani
*
template <class T, int N>
class Mult<CFSliceNMatrix<T,N>,CFSliceNMatrix<T,N>,T,N> : public NExpr< Mult<CFSliceNMatrix<T,N>,CFSliceNMatrix<T,N>,T,N>,T,N> {
public:
  Mult(const NExpr<CFSliceNMatrix<T,N>,T,N>& v1, const NExpr<CFSliceNMatrix<T,N>,T,N>& v2) :
    NExpr<Mult<CFSliceNMatrix<T,N>,CFSliceNMatrix<T,N>,T,N>, T,N>(*this),
    ex1(v1), ex2(v2) {}

  T at(Uint i) const
  {
    T res = T();
    const Uint jc = i%ex2.getData().nbCols();
    for (Uint j = 0, k = (i/ex2.getData().nbCols())*ex1.getData().nbCols();
         j < ex1.getData().nbCols(); ++j, ++k) {
      res += ex1.at(k)*ex2.at(jc + ex2.getData().nbCols()*j);
    }
    return res;
  }

  Uint size() const {return ex1.getData().nbRows()*ex2.getData().nbCols();}

private:
  const NExpr<CFSliceNMatrix<T,N>, T,N>& ex1;
  const NExpr<CFSliceNMatrix<T,N>, T,N>& ex2;
};

* Partial specialization of the previous expression template class
* for multiplication of the type CFSliceNMatrix*CFSliceNVector*CFSliceNMatrix,
* being CFSliceNVector a diagonal matrix.
* The expression template accepts one parameter, namely the
* built-in type of both the expressions (double, float, etc.).
*
* The constructor accepts as arguments two references, one to
* a CFSliceNMatrix and the other one to an expression template object
* corresponding to the CFSliceNVector*CFSliceNMatrix product.
*
* @author Andrea Lani
*
template <class T, int N>
class Mult<CFSliceNMatrix<T,N>,Mult<CFSliceNVector<T,N>,CFSliceNMatrix<T,N>,T,N>,T,N> :
  public NExpr<Mult<CFSliceNMatrix<T,N>,Mult<CFSliceNVector<T,N>,CFSliceNMatrix<T,N>,T,N>,T,N>,T,N> {
public:
  Mult(const NExpr<CFSliceNMatrix<T,N>,T,N>& v1,
      const NExpr<Mult<CFSliceNVector<T,N>,CFSliceNMatrix<T,N>,T,N>,T,N>& v2) :
    NExpr<Mult<CFSliceNMatrix<T,N>,Mult<CFSliceNVector<T,N>,CFSliceNMatrix<T,N>,T,N>,T,N>,T,N>(*this),
    ex1(v1), ex2(v2) {}

  T at(Uint i) const
  {
    T res = T();
    const Uint jc = i%ex2.getData().ex2.getData().nbCols();
    for (Uint j = 0, k = (i/ex2.getData().ex2.getData().nbCols())*ex1.getData().nbCols();
  j < ex1.getData().nbCols(); ++j, ++k) {
      res += ex1.at(k)*ex2.getData().ex2.at(jc + ex2.getData().ex2.getData().nbCols()*j)*ex2.getData().ex1.at(j);
    }
    return res;
  }

  Uint size() const
  {
    return ex1.getData().nbRows()*
      ex2.getData().ex2.getData().nbCols();
  }

private:
  const NExpr<CFSliceNMatrix<T,N>, T,N>& ex1;
  const NExpr<Mult<CFSliceNVector<T,N>,CFSliceNMatrix<T,N>,T,N>,T,N>& ex2;
};

////////////////////////////////////////////////////////////////////////////////

* Inline function (the keyword must be put otherwise the compiler will not inline it,
* as shown by the profiler) providing the overloading of the basic binary operators
* between two expression template objects.
* @param v1 expression template object (first operand)
* @param v2 expression template object (second operand)
#define OVERLOAD_BINARY_OP(__OpName__,__op__) \
  template <class V1, class V2, class T, int N>          \
  inline __OpName__<V1, V2, T,N> operator __op__ (const NExpr<V1,T,N>& v1, const NExpr<V2,T,N>& v2) \
{\
  return __OpName__<V1,V2,T,N>(v1,v2);    \
}

OVERLOAD_BINARY_OP(Add,+)
OVERLOAD_BINARY_OP(Sub,-)
OVERLOAD_BINARY_OP(Mult,*)
OVERLOAD_BINARY_OP(Div,/)

#undef OVERLOAD_BINARY_OP

* Inline function (the keyword must be put otherwise the compiler will not inline it,
* as shown by the profiler) providing the overloading of the basic binary operators
* between a constant and an expression template object.
* @param v1 constant value (first operand)
* @param v2 expression template object (second operand)
#define OVERLOAD_BINARY_OP_CONST1(__OpName__,__op__)    \
  template <class V2, class T, int N>          \
  inline __OpName__<T, V2, T,N> operator __op__ (const T& v1, const NExpr<V2,T,N>& v2) \
{\
  return __OpName__<T,V2,T,N>(v1,v2);    \
}

OVERLOAD_BINARY_OP_CONST1(Add,+)
OVERLOAD_BINARY_OP_CONST1(Sub,-)
OVERLOAD_BINARY_OP_CONST1(Mult,*)
OVERLOAD_BINARY_OP_CONST1(Div,/)

#undef OVERLOAD_BINARY_OP_CONST1

* Inline function (the keyword must be put otherwise the compiler will not inline it,
* as shown by the profiler) providing the overloading of the basic binary operators
* between an expression template object and a constant.
* @param v1 constant value (first operand)
* @param v2 expression template object (second operand)
#define OVERLOAD_BINARY_OP_CONST2(__OpName__,__op__) \
  template <class V1, class T, int N>          \
  inline __OpName__<V1, T, T,N> operator __op__ (const NExpr<V1,T,N>& v1, const T& v2) \
{\
  return __OpName__<V1,T,T,N>(v1,v2);    \
}

OVERLOAD_BINARY_OP_CONST2(Add,+)
OVERLOAD_BINARY_OP_CONST2(Sub,-)
OVERLOAD_BINARY_OP_CONST2(Mult,*)
OVERLOAD_BINARY_OP_CONST2(Div,/)

#undef OVERLOAD_BINARY_OP_CONST2

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

}   // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif
