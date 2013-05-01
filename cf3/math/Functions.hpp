// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_Functions_hpp
#define cf3_Math_Functions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CF.hpp"
#include "math/LibMath.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  namespace math {

////////////////////////////////////////////////////////////////////////////////

/// @brief Static functions for various useful operations
/// @author Andrea Lani
/// @author Tiago Quintino
/// @author Mehmet Sarp Yalim
namespace Functions
{
  /// Signum function
  /// @param value the real to which infer the sign
  /// @return -1.0 if value < 0.0
  /// @return 0.0 if value = 0.0
  /// @return 1.0 if value > 0.0
  Real signum (const Real& value);

  /// Sign function
  /// @param value the real to which infer the sign
  /// @return -1.0 if value < 0.0
  /// @return 1.0 if value >= 0.0
  Real sign(const Real& value);

  /// Change the sign of the first argument with the sign
  /// of the second argument
  /// @param   value   the value of this will be returned
  /// @param   newSignValue   the sign of what will be returned
  /// @return  the value with the sign of newSignValue
  Real change_sign(const Real& value, const Real& newSignValue);

  /// Heavyside function
  /// @param value is real
  /// @return 1.0 if value > 0.0
  /// @return 0.0 if value < 0.0
  /// @return 0.5 if value = 0.0
  Real heavyside(const Real& value);

  /// Calculate the euclidean distance between two "points"
  /// @pre T1 and T2 must have the overloading of the operator[] implemented
  template <class T1, class T2>
  Real get_distance(const T1& n1, const T2& n2);

  /// Calculate the factorial
  /// @param   n   calculate factorial of this number
  /// @return  factorial of n
  Uint factorial(const Uint& n);

  /// Mixed Product of three vectors
  /// @pre size() == 3 == v1.size() == v2.size() == v3.size() == temp.size()
  ///      (cf3_assertion can check this)
  /// @param v1   first vector
  /// @param v2   second vector
  /// @param v3   third vector
  /// @param temp temporary vector
  /// @return the mixed product
  template <class T1, class T2, class T3, class T4>
  Real mixed_product (const T1& v1,
                      const T2& v2,
                      const T3& v3,
                      T4& temp);

  /// Cross Product for vector*vector operations
  /// @pre v1.size() == v2.size() == result.size() == 3
  /// @param v1 first vector
  /// @param v2 second vector
  /// @param result vector storing the result
  template <class T1, class T2, class T3>
  void cross_product (const T1& v1,
                      const T2& v2,
                      T3& result);

  /// Internal Product for vector*vector operations
  /// \f$ s = v \cdot v1 \f$.
  /// Objects must be of same size.
  /// @param v1 first vector
  /// @param v2 second vector
  /// @return the inner product of the two given vectors
  template <class T1, class T2>
  Real inner_product (const T1& v1, const T2& v2);

  /// Tensor Product for vector*vector operations
  /// \f$ s = v \cdot v1 \f$.
  /// @param v1 [in]  first vector
  /// @param v2 [in]  second vector
  /// @param v3 [out] the tensor product of the two given vectors
  template <class T1, class T2, class T3>
  void tensor_product(const T1& v1, const T2& v2, T3& m);


  ////////////////////////////////////////////////////////////////////////////////

  inline Real signum (const Real& value)
  {
    if ( value <  0.0 )  return -1.0;
    if ( value == 0.0 )  return 0.0;
    return 1.0;
  }


  inline Real sign(const Real& value)
  {
    if (value < 0.0) return -1.0;
    else return 1.0;
  }


  inline Real change_sign(const Real& value, const Real& newSignValue)
  {
    return newSignValue >= 0 ? (value >=0 ? value : -value) : (value >= 0 ? -value : value);
  }


  inline Real heavyside(const Real& value)
  {
    if (value < 0.0) return 0.0;
    if (value == 0.0) return 0.5;
    return 1.0;
  }


  template <class T1, class T2>
  inline Real get_distance(const T1& n1, const T2& n2)
  {
    cf3_assert(n1.size() == n2.size());

    Real dist = 0.;
    const Uint size =  n1.size();
    for (Uint i = 0; i < size; ++i)
    {
      const Real diff = n1[i] - n2[i];
      dist += diff*diff;
    }
    return std::sqrt(dist);
  }


  inline Uint factorial(const Uint& n)
  {
    if (n<2)
      return 1;
    else
      return (n*factorial(n-1));
  }


  template <class T1, class T2, class T3, class T4>
  inline Real mixed_product (const T1& v1,
                             const T2& v2,
                             const T3& v3,
                             T4& temp)
  {
    // sanity checks
    cf3_assert(v1.size() == 3);
    cf3_assert(v2.size() == 3);
    cf3_assert(v3.size() == 3);
    cf3_assert(temp.size() == 3);

    cross_product(v1, v2, temp);
    return inner_product(v3, temp);
  }

  template <class T1, class T2, class T3>
  inline void cross_product (const T1& v1,
                             const T2& v2,
                             T3& result)
  {
    // sanity checks
    cf3_assert(v1.size() == 3);
    cf3_assert(v2.size() == 3);
    cf3_assert(result.size() == 3);

    result[0] =  v1[1]*v2[2] - v1[2]*v2[1];
    result[1] = -v1[0]*v2[2] + v1[2]*v2[0];
    result[2] =  v1[0]*v2[1] - v1[1]*v2[0];
  }


  template <class T1, class T2>
  inline Real inner_product (const T1& v1, const T2& v2)
  {
    cf3_assert(v1.size() == v2.size());

    const Uint size = v1.size();
    Real result = 0.0;
    for (Uint i = 0; i < size; ++i)
      result += v1[i]*v2[i];
    return result;
  }


  template <class T1, class T2, class T3>
  inline void tensor_product(const T1& v1, const T2& v2, T3& m)
  {
    cf3_assert(m.getNbRows()    == v1.size());
    cf3_assert(m.getNbColumns() == v2.size());

    const Uint v1size = v1.size();
    const Uint v2size = v2.size();
    for (Uint i = 0; i < v1size; ++i) {
      for (Uint j = 0; j < v2size; ++j) {
        m(i,j) = v1[i]*v2[j];
      }
    }
  }



} // end namespace Functions

////////////////////////////////////////////////////////////////////////////////

  } // math

} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Math_Functions_hpp
