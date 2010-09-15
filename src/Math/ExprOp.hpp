// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_ExprOp_hpp
#define CF_Math_ExprOp_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

    template <class T> class MatrixT;
    template <class T> class VectorT;
    template <class T> class VectorSliceT;
    template <class T> class MatrixSliceT;

////////////////////////////////////////////////////////////////////////////////

/// Definition of a wrapper base expression class Expr, the closure objects
/// (derived from Expr using the CRTP technique), some partial or
/// explicit specializations of the closure objects to treat special cases
/// and definition of the overloading functions accepting expressions or constants
/// as arguments.
/// @author Andrea Lani
template <class DATA, class T>
class Expr {
public:

  Expr(const DATA& data) : m_exdata(data) {}

  T at (const Uint& i) const {return m_exdata.at(i);}

  Uint size() const {return m_exdata.size();}

  const DATA& getData() const {return m_exdata;}

private:
  const DATA& m_exdata;
};

/// Definition of an expression template class for basic binary operations.
/// A macro is used to save tedious code duplication.
/// The expression template accepts three parameters:
/// 1. first operand expression type
/// 2. second operand expression type
/// 3. built-in type of both the expressions (double, float, etc.).
/// The constructor accepts as arguments two references to expression
/// template objects (corrresponding to the two operands of the expression).
/// @author Andrea Lani
#define EXPR_BINARY_OP(__OpName__,__op__)    \
template <class V1, class V2, class T>        \
class __OpName__ : public Expr< __OpName__<V1,V2,T>, T> {  \
public:            \
  __OpName__(const Expr<V1,T>& v1, const Expr<V2,T>& v2) :  \
    Expr< __OpName__<V1,V2,T>,T >(*this),      \
    ex1(v1), ex2(v2) {}             \
                     \
  T at (const Uint& i) const             \
  {              \
    return ex1.at(i) __op__ ex2.at(i);      \
  }              \
                \
  Uint size() const {return ex1.size();}    \
                \
  const Expr<V1,T>& ex1;        \
  const Expr<V2,T>& ex2;        \
};

EXPR_BINARY_OP(Add,+)
EXPR_BINARY_OP(Sub,-)
EXPR_BINARY_OP(Mult,*)
EXPR_BINARY_OP(Div,/)

#undef EXPR_BINARY_OP

/// Partial specialization of the previous expression template class
/// for basic binary operations between a constant and an expression.
/// A macro is used to save tedious code duplication.
/// The expression template accepts two parameters:
/// 2. second operand expression type
/// 3. built-in type of both the expressions (double, float, etc.).
/// The constructor accepts as arguments two references, one to constant,
/// the other to an expression template object.
///
/// @author Andrea Lani
#define EXPR_BINARY_OP_CONST1(__OpName__,__op__)      \
template <class V2, class T>        \
class __OpName__<T,V2,T> : public Expr< __OpName__<T,V2,T>, T> {  \
public:            \
  __OpName__(const T& v1, const Expr<V2,T>& v2) :  \
    Expr< __OpName__<T,V2,T>,T >(*this),      \
    ex1(v1), ex2(v2) {}             \
                     \
  T at(const Uint& i) const             \
  {              \
    return ex1 __op__ ex2.at(i);      \
  }              \
      \
  Uint size() const {return ex2.size();}    \
      \
private:            \
const T& ex1;            \
  const Expr<V2,T>& ex2;        \
};

EXPR_BINARY_OP_CONST1(Add,+)
EXPR_BINARY_OP_CONST1(Sub,-)
EXPR_BINARY_OP_CONST1(Mult,*)
EXPR_BINARY_OP_CONST1(Div,/)

#undef EXPR_BINARY_OP_CONST1

/// Partial specialization of the previous expression template class
/// for basic binary operations between an expression and a constant.
/// A macro is used to save tedious code duplication.
/// The expression template accepts two parameters:
/// 2. first operand expression type
/// 3. built-in type of both the expressions (double, float, etc.).
/// The constructor accepts as arguments two references, one to an
/// expression template object, the other to a constant.
/// @author Andrea Lani
#define EXPR_BINARY_OP_CONST2(__OpName__,__op__)      \
template <class V1, class T>        \
class __OpName__<V1,T,T> : public Expr< __OpName__<V1,T,T>, T> {  \
public:            \
  __OpName__(const Expr<V1,T>& v1, const T& v2) :  \
    Expr< __OpName__<V1,T,T>,T >(*this),      \
    ex1(v1), ex2(v2) {}             \
                     \
  T at(const Uint& i) const          \
  {              \
    return ex1.at(i) __op__ ex2;      \
  }              \
      \
  Uint size() const {return ex1.size();}    \
      \
private:            \
  const Expr<V1,T>& ex1;          \
  const T&          ex2;        \
};

EXPR_BINARY_OP_CONST2(Add,+)
EXPR_BINARY_OP_CONST2(Sub,-)
EXPR_BINARY_OP_CONST2(Mult,*)
EXPR_BINARY_OP_CONST2(Div,/)

#undef EXPR_BINARY_OP_CONST2

/// Partial specialization of the previous expression template class
/// for multiplication between a MatrixT and another type
/// (e.g. VectorT or expressions involving VectorTs).
/// The expression template accepts two parameters:
/// 2. second operand expression type
/// 3. built-in type of both the expressions (double, float, etc.).
/// The constructor accepts as arguments two references, one to a
/// MatrixT expression template object, the other to an expression template
/// object.
///
/// @author Andrea Lani
template <class V2, class T>
class Mult<MatrixT<T>,V2,T> : public Expr< Mult<MatrixT<T>,V2,T>,T> {
public:
  Mult(const Expr<MatrixT<T>,T>& v1, const Expr<V2,T>& v2) :
    Expr<Mult<MatrixT<T>,V2,T>, T>(*this),
    ex1(v1), ex2(v2) {}

  T at(const Uint& i) const
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
  const Expr<MatrixT<T>, T>& ex1;
  const Expr<V2,T>&         ex2;
};

/// Partial specialization of the previous expression template class
/// for multiplication between two MatrixT's.
/// The expression template accepts one parameter, namely the
/// built-in type of both the expressions (double, float, etc.).
///
/// The constructor accepts as arguments two references to
/// MatrixT expression template objects.
///
/// @author Andrea Lani
template <class T>
class Mult<MatrixT<T>,MatrixT<T>,T> : public Expr< Mult<MatrixT<T>,MatrixT<T>,T>,T> {
  friend class MatrixT<T>;
public:
  Mult(const Expr<MatrixT<T>,T>& v1, const Expr<MatrixT<T>,T>& v2) :
    Expr<Mult<MatrixT<T>,MatrixT<T>,T>, T>(*this),
    ex1(v1), ex2(v2) {}

  T at(const Uint& i) const
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
  const Expr<MatrixT<T>, T>& ex1;
  const Expr<MatrixT<T>, T>& ex2;
};

/// Partial specialization of the previous expression template class
/// for multiplication of the type MatrixT*VectorT*MatrixT,
/// being VectorT a diagonal matrix.
/// The expression template accepts one parameter, namely the
/// built-in type of both the expressions (double, float, etc.).
///
/// The constructor accepts as arguments two references, one to
/// a MatrixT and the other one to an expression template object
/// corresponding to the VectorT-MatrixT product.
///
/// @author Andrea Lani
template <class T>
class Mult<MatrixT<T>,Mult<VectorT<T>,MatrixT<T>,T>,T> :
  public Expr<Mult<MatrixT<T>,Mult<VectorT<T>,MatrixT<T>,T>,T>,T> {

  friend class MatrixT<T>;
public:
  Mult(const Expr<MatrixT<T>,T>& v1,
      const Expr<Mult<VectorT<T>,MatrixT<T>,T>,T>& v2) :
    Expr<Mult<MatrixT<T>,Mult<VectorT<T>,MatrixT<T>,T>,T>,T>(*this),
    ex1(v1), ex2(v2) {}

  T at(const Uint& i) const
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
  const Expr<MatrixT<T>, T>& ex1;
  const Expr<Mult<VectorT<T>,MatrixT<T>,T>,T>& ex2;
};

/// Partial specialization of the previous expression template class
/// for multiplication between a MatrixSliceT and another type
/// (e.g. VectorT or expressions involving VectorTs).
/// The expression template accepts two parameters:
/// 2. second operand expression type
/// 3. built-in type of both the expressions (double, float, etc.).
///
/// The constructor accepts as arguments two references, one to a
/// MatrixSliceT expression template object, the other to an expression template
/// object.
///
/// @author Andrea Lani
template <class V2, class T>
class Mult<MatrixSliceT<T>,V2,T> : public Expr< Mult<MatrixSliceT<T>,V2,T>,T> {
public:
  Mult(const Expr<MatrixSliceT<T>,T>& v1, const Expr<V2,T>& v2) :
    Expr<Mult<MatrixSliceT<T>,V2,T>, T>(*this),
    ex1(v1), ex2(v2) {}

  T at(const Uint& i) const
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
  const Expr<MatrixSliceT<T>, T>& ex1;
  const Expr<V2,T>&         ex2;
};

/// Partial specialization of the previous expression template class
/// for multiplication between two MatrixSliceT's.
/// The expression template accepts one parameter, namely the
/// built-in type of both the expressions (double, float, etc.).
///
/// The constructor accepts as arguments two references to
/// MatrixSliceT expression template objects.
///
/// @author Andrea Lani
template <class T>
class Mult<MatrixSliceT<T>,MatrixSliceT<T>,T> : public Expr< Mult<MatrixSliceT<T>,MatrixSliceT<T>,T>,T> {
public:
  Mult(const Expr<MatrixSliceT<T>,T>& v1, const Expr<MatrixSliceT<T>,T>& v2) :
    Expr<Mult<MatrixSliceT<T>,MatrixSliceT<T>,T>, T>(*this),
    ex1(v1), ex2(v2) {}

  T at(const Uint& i) const
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
  const Expr<MatrixSliceT<T>, T>& ex1;
  const Expr<MatrixSliceT<T>, T>& ex2;
};

/// Partial specialization of the previous expression template class
/// for multiplication of the type MatrixSliceT*VectorSliceT*MatrixSliceT,
/// being VectorSliceT a diagonal matrix.
/// The expression template accepts one parameter, namely the
/// built-in type of both the expressions (double, float, etc.).
///
/// The constructor accepts as arguments two references, one to
/// a MatrixSliceT and the other one to an expression template object
/// corresponding to the VectorSliceT*MatrixSliceT product.
///
/// @author Andrea Lani
template <class T>
class Mult<MatrixSliceT<T>,Mult<VectorSliceT<T>,MatrixSliceT<T>,T>,T> :
  public Expr<Mult<MatrixSliceT<T>,Mult<VectorSliceT<T>,MatrixSliceT<T>,T>,T>,T> {
public:
  Mult(const Expr<MatrixSliceT<T>,T>& v1,
      const Expr<Mult<VectorSliceT<T>,MatrixSliceT<T>,T>,T>& v2) :
    Expr<Mult<MatrixSliceT<T>,Mult<VectorSliceT<T>,MatrixSliceT<T>,T>,T>,T>(*this),
    ex1(v1), ex2(v2) {}

  T at(const Uint& i) const
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
  const Expr<MatrixSliceT<T>, T>& ex1;
  const Expr<Mult<VectorSliceT<T>,MatrixSliceT<T>,T>,T>& ex2;
};

////////////////////////////////////////////////////////////////////////////////

/// Inline function (the keyword must be put otherwise the compiler will not inline it,
/// as shown by the profiler) providing the overloading of the basic binary operators
/// between two expression template objects.
/// @param v1 expression template object (first operand)
/// @param v2 expression template object (second operand)
#define OVERLOAD_BINARY_OP(__OpName__,__op__) \
  template <class V1, class V2, class T>          \
  inline __OpName__<V1, V2, T> operator __op__ (const Expr<V1,T>& v1, const Expr<V2,T>& v2) \
{\
  return __OpName__<V1,V2,T>(v1,v2);    \
}

OVERLOAD_BINARY_OP(Add,+)
OVERLOAD_BINARY_OP(Sub,-)
OVERLOAD_BINARY_OP(Mult,*)
OVERLOAD_BINARY_OP(Div,/)

#undef OVERLOAD_BINARY_OP

/// Inline function (the keyword must be put otherwise the compiler will not inline it,
/// as shown by the profiler) providing the overloading of the basic binary operators
/// between a constant and an expression template object.
///
/// @param v1 constant value (first operand)
/// @param v2 expression template object (second operand)
#define OVERLOAD_BINARY_OP_CONST1(__OpName__,__op__)    \
  template <class V2, class T>          \
  inline __OpName__<T, V2, T> operator __op__ (const T& v1, const Expr<V2,T>& v2) \
{\
  return __OpName__<T,V2,T>(v1,v2);    \
}

OVERLOAD_BINARY_OP_CONST1(Add,+)
OVERLOAD_BINARY_OP_CONST1(Sub,-)
OVERLOAD_BINARY_OP_CONST1(Mult,*)
OVERLOAD_BINARY_OP_CONST1(Div,/)

#undef OVERLOAD_BINARY_OP_CONST1

/// Inline function (the keyword must be put otherwise the compiler will not inline it,
/// as shown by the profiler) providing the overloading of the basic binary operators
/// between an expression template object and a constant.
/// @param v1 constant value (first operand)
/// @param v2 expression template object (second operand)
#define OVERLOAD_BINARY_OP_CONST2(__OpName__,__op__) \
  template <class V1, class T>          \
  inline __OpName__<V1, T, T> operator __op__ (const Expr<V1,T>& v1, const T& v2) \
{\
  return __OpName__<V1,T,T>(v1,v2);    \
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
