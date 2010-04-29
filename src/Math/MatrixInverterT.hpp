#ifndef CF_Math_MatrixInverterT_hpp
#define CF_Math_MatrixInverterT_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/LUInverterT.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// This class inverts a generic matrix.
/// It takes a parameter with the size of the square matrix.
/// It can be specialised for small sizes.
///
/// @author Andrea Lani
/// @author Tiago Quintino
template < unsigned int SIZE >
struct MatrixInverterT {

  /// Invert the given matrix a and put the result in x
  void invert (const RealMatrix& a, RealMatrix& x)
  {
    m_inverter.invert(a,x);
  }

private: // data

  /// default inverter algorithm is LU
  LUInverterT <SIZE> m_inverter;

}; // class MatrixInverterT

////////////////////////////////////////////////////////////////////////////////

/// Specialization of matrix inversion for SIZE 2
/// @author Tiago Quintino
template <> struct MatrixInverterT <2> {

  /// Invert the given matrix a and put the result in x
  void invert (const RealMatrix& a, RealMatrix& x)
  {
    cf_assert(a.nbRows() == 2);
    cf_assert(a.nbCols() == 2);
    cf_assert(x.nbRows() == 2);
    cf_assert(x.nbCols() == 2);

    const Real det = a.determ2();
    cf_assert(MathChecks::isNotZero(det));
    const Real invDet = 1./det;

    x[0] =  a[3]*invDet;
    x[1] = -a[1]*invDet;
    x[2] = -a[2]*invDet;
    x[3] =  a[0]*invDet;
  }

}; // class MatrixInverterT<2>

////////////////////////////////////////////////////////////////////////////////

/// Specialization of matrix inversion for SIZE 3
/// @author Tiago Quintino
template <> struct MatrixInverterT <3> {

  /// Invert the given matrix a and put the result in x
  void invert (const RealMatrix& a, RealMatrix& x)
  {
    cf_assert(a.nbRows() == 3);
    cf_assert(a.nbCols() == 3);
    cf_assert(x.nbRows() == 3);
    cf_assert(x.nbCols() == 3);

    const Real det = a.determ3();
    cf_assert(MathChecks::isNotZero(det));
    const Real invDet = 1./det;

    x[0] =  (a[4]*a[8] - a[5]*a[7])*invDet;
    x[1] = -(a[1]*a[8] - a[2]*a[7])*invDet;
    x[2] =  (a[1]*a[5] - a[4]*a[2])*invDet;
    x[3] = -(a[3]*a[8] - a[5]*a[6])*invDet;
    x[4] =  (a[0]*a[8] - a[2]*a[6])*invDet;
    x[5] = -(a[0]*a[5] - a[2]*a[3])*invDet;
    x[6] =  (a[3]*a[7] - a[4]*a[6])*invDet;
    x[7] = -(a[0]*a[7] - a[1]*a[6])*invDet;
    x[8] =  (a[0]*a[4] - a[1]*a[3])*invDet;
  }

}; // class MatrixInverterT<3>

////////////////////////////////////////////////////////////////////////////////

/// Specialization of matrix inversion for SIZE 4
/// @author Tiago Quintino
template <> struct MatrixInverterT <4> {

  /// Invert the given matrix a and put the result in x
  void invert (const RealMatrix& a, RealMatrix& x)
  {
    cf_assert(a.nbRows() == 4);
    cf_assert(a.nbCols() == 4);
    cf_assert(x.nbRows() == 4);
    cf_assert(x.nbCols() == 4);

    const Real t14 = a[0]*a[5] ;
    const Real t15 = a[10]*a[15] ;
    const Real t17 = a[11]*a[14] ;
    const Real t19 = a[0]*a[9] ;
    const Real t20 = a[6]*a[15] ;
    const Real t22 = a[7]*a[14] ;
    const Real t24 = a[0]*a[13] ;
    const Real t25 = a[6]*a[11] ;
    const Real t27 = a[7]*a[10] ;
    const Real t29 = a[4]*a[1] ;
    const Real t32 = a[4]*a[9] ;
    const Real t33 = a[2]*a[15] ;
    const Real t35 = a[3]*a[14] ;
    const Real t37 = a[4]*a[13] ;
    const Real t38 = a[2]*a[11] ;
    const Real t40 = a[3]*a[10] ;
    const Real t42 = t14*t15-t14*t17-t19*t20+t19*t22+t24*t25-t24*t27-t29*t15+t29*t17+t32*t33-t32*t35-t37*t38+t37*t40;
    const Real t43 = a[8]*a[1] ;
    const Real t46 = a[8]*a[5] ;
    const Real t49 = a[8]*a[13] ;
    const Real t50 = a[2]*a[7] ;
    const Real t52 = a[3]*a[6] ;
    const Real t54 = a[12]*a[1] ;
    const Real t57 = a[12]*a[5] ;
    const Real t60 = a[12]*a[9] ;
    const Real t63 = t43*t20-t43*t22-t46*t33+t46*t35+t49*t50-t49*t52-t54*t25+t54*t27+t57*t38-t57*t40-t60*t50+t60*t52;

    const Real ddet = t42+t63 ;
    cf_assert(MathChecks::isNotZero(ddet));
    const Real deter = 1. / ddet ;

    const Real t71  = a[9]*a[2] ;
    const Real t73  = a[9]*a[3] ;
    const Real t75  = a[13]*a[2] ;
    const Real t77  = a[13]*a[3] ;
    const Real t81  = a[1]*a[6] ;
    const Real t83  = a[1]*a[7] ;
    const Real t85  = a[5]*a[2] ;
    const Real t87  = a[5]*a[3] ;
    const Real t119 = a[8]*a[2] ;
    const Real t121 = a[8]*a[3] ;
    const Real t123 = a[12]*a[2] ;
    const Real t125 = a[12]*a[3] ;
    const Real t129 = a[0]*a[6] ;
    const Real t131 = a[0]*a[7] ;
    const Real t133 = a[4]*a[2] ;
    const Real t135 = a[4]*a[3] ;

    x[0] = (a[5]*a[10]*a[15]-a[5]*a[11]*a[14]-a[9]*a[6]*a[15]
          +  a[9]*a[7]*a[14]+a[13]*a[6]*a[11]-a[13]*a[7]*a[10])*deter ;
    x[1] = -(a[1]*a[10]*a[15]-a[1]*a[11]*a[14]-t71*a[15]+t73*a[14]
        +   t75*a[11]-t77*a[10])*deter ;
    x[2] = (t81*a[15]-t83*a[14]-t85*a[15]+t87*a[14]+t75*a[7]-t77*a[6])*deter ;
    x[3] = -(t81*a[11]-t83*a[10]-t85*a[11]+t87*a[10]+t71*a[7]-t73*a[6])*deter ;

    x[4] = (-a[4]*a[10]*a[15]+a[4]*a[11]*a[14]+a[8]*a[6]*a[15]
                -   a[8]*a[7]*a[14]-a[12]*a[6]*a[11]+a[12]*a[7]*a[10])*deter ;
    x[5] = (a[0]*a[10]*a[15]-a[0]*a[11]*a[14]-t119*a[15]+t121*a[14]
                +  t123*a[11]-t125*a[10])*deter ;
    x[6] = -(t129*a[15]-t131*a[14]-t133*a[15]+t135*a[14]+t123*a[7]-t125*a[6])*deter ;
    x[7] = (t129*a[11]-t131*a[10]-t133*a[11]+t135*a[10]+t119*a[7]-t121*a[6])*deter ;

    x[8] = -(-t32*a[15]+t37*a[11]+t46*a[15]-t49*a[7]-t57*a[11]+t60*a[7])*deter ;
    x[9] = -(t19*a[15]-t24*a[11]-t43*a[15]+t121*a[13]+t54*a[11]-t125*a[9])*deter ;
    x[10] = (t14*a[15]-t24*a[7]-t29*a[15]+t135*a[13]+t54*a[7]-t125*a[5])*deter ;
    x[11] = -(t14*a[11]-t19*a[7]-t29*a[11]+t135*a[9]+t43*a[7]-t121*a[5])*deter ;

    x[12] = -(t32*a[14]-t37*a[10]-t46*a[14]+t49*a[6]+t57*a[10]-t60*a[6])*deter ;
    x[13] = (t19*a[14]-t24*a[10]-t43*a[14]+t119*a[13]+t54*a[10]-t123*a[9])*deter ;
    x[14] = -(t14*a[14]-t24*a[6]-t29*a[14]+t133*a[13]+t54*a[6]-t123*a[5])*deter ;
    x[15] = (t14*a[10]-t19*a[6]-t29*a[10]+t133*a[9]+t43*a[6]-t119*a[5])*deter ;
  }

}; // class MatrixInverterT<4>

////////////////////////////////////////////////////////////////////////////////

  } // namespace Math

} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_MatrixInverterT_hpp
