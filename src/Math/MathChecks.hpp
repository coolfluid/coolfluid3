// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_MathChecks_hpp
#define CF_Math_MathChecks_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/NonInstantiable.hpp"

#include "Math/BoostMath.hpp"
#include "Math/MathConsts.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {
	
////////////////////////////////////////////////////////////////////////////////

/// Provides an a set of static functions for checking Real numbers.
/// This class is not instantiable.
/// @author Tiago Quintino
namespace MathChecks
{
  /// Function to check if two Real numbers are equal.
  /// @param x
  /// @param y
  /// @return true if equal or almost equal within the accepted error fuzz.
  bool isEqual(const Real& x,  const Real& y);

  /// Function to check if two Real numbers are not equal.
  /// @param x
  /// @param y
  /// @return true if not equal or almost unequal within the accepted error fuzz.
  bool isNotEqual(const Real& x,  const Real& y);

  /// Function to check if two Real numbers are equal.
  /// @param x
  /// @param y
  /// @param fuzz
  /// @return true if equal or almost equal within the accepted error fuzz.
  bool isEqualWithError(const Real& x, const Real& y, const Real& fuzz);
	
  /// Function to check if two Real numbers are not equal.
  /// @param x
  /// @param y
  /// @param fuzz
  /// @return true if not equal or almost unequal within the accepted error fuzz.
  bool isNotEqualWithError(const Real& x, const Real& y, const Real& fuzz);

  /// Function to check if a Real number is finite number. This means is not a NaN neither a INF
  /// @param x
  /// @return true if x is finite
  bool isFinite(const Real& x);

  /// Function to check if a Real number is either minus or plus INF
  /// @param x
  /// @return true if x is finite
  bool isInf(const Real& x);

  /// Function to check if a Real number is a NaN (Not a Number)
  /// @param x
  /// @return true if x is a NaN
  bool isNaN(const Real& x);

  /// Function to check if a Real number is zero or very close.
  /// @param x
  /// @return true if equal to zero or almost equal within the accepted error fuzz.
  bool isZeroWithError(const Real& x, const Real& fuzz);

  /// Function to check if a Real number is not zero or very close.
  /// @param x
  /// @return true if not equal to zero or not almost equal within the accepted error fuzz.
  bool isNotZeroWithError(const Real& x, const Real& fuzz);

  /// Function to check if a Real number is zero or very close.
  /// @param x
  /// @return true if equal to zero or almost equal within the accepted error fuzz.
  bool isZero(const Real& x);

  /// Function to check if a Real number is not zero or very close.
  /// @param x
  /// @return true if not equal to zero or not almost equal within the accepted error fuzz.
  bool isNotZero(const Real& x);

  /// Checks is real is positive.
  /// Kind of a sign function returning a bool.
  bool isPositive(const Real& value);

  /// Checks is real is negative.
  /// Kind of a sign function returning a bool.
  bool isNegative(const Real& value);

	
	////////////////////////////////////////////////////////////////////////////////

		
	/// Function to check if two Real numbers are equal.
  /// @param x
  /// @param y
  /// @return true if equal or almost equal within the accepted error fuzz.
  inline bool isEqual(const Real& x,  const Real& y)
  {
    return isEqualWithError(x,y,MathConsts::RealMin);
  }
	
  /// Function to check if two Real numbers are not equal.
  /// @param x
  /// @param y
  /// @return true if not equal or almost unequal within the accepted error fuzz.
  inline bool isNotEqual(const Real& x,  const Real& y)
  {
    return isNotEqualWithError(x,y,MathConsts::RealMin);
  }
	
  /// Function to check if two Real numbers are equal.
  /// @param x
  /// @param y
  /// @param fuzz
  /// @return true if equal or almost equal within the accepted error fuzz.
  inline bool isEqualWithError(const Real& x, const Real& y, const Real& fuzz)
  {
    // see Knuth section 4.2.2 pages 217-218
    return std::abs(x - y) <= fuzz * std::abs(x);
  }
	
  /// Function to check if two Real numbers are not equal.
  /// @param x
  /// @param y
  /// @param fuzz
  /// @return true if not equal or almost unequal within the accepted error fuzz.
  inline bool isNotEqualWithError(const Real& x, const Real& y, const Real& fuzz)
  {
    return !isEqualWithError(x,y,fuzz);
  }
	
  /// Function to check if a Real number is finite number. This means is not a NaN neither a INF
  /// @param x
  /// @return true if x is finite
  inline bool isFinite(const Real& x)
  {
    return (boost::math::isfinite)(x);
  }
	
  /// Function to check if a Real number is either minus or plus INF
  /// @param x
  /// @return true if x is finite
  inline bool isInf(const Real& x)
  {
    return (boost::math::isinf)(x);
  }
	
  /// Function to check if a Real number is a NaN (Not a Number)
  /// @param x
  /// @return true if x is a NaN
  inline bool isNaN(const Real& x)
  {
    return (boost::math::isnan)(x);
  }
	
  /// Function to check if a Real number is zero or very close.
  /// @param x
  /// @return true if equal to zero or almost equal within the accepted error fuzz.
  inline bool isZeroWithError(const Real& x, const Real& fuzz)
  {
    return std::abs(x) <= fuzz;
  }
	
	
  /// Function to check if a Real number is not zero or very close.
  /// @param x
  /// @return true if not equal to zero or not almost equal within the accepted error fuzz.
  inline bool isNotZeroWithError(const Real& x, const Real& fuzz)
  {
    return !isZeroWithError(x,fuzz);
  }
	
  /// Function to check if a Real number is zero or very close.
  /// @param x
  /// @return true if equal to zero or almost equal within the accepted error fuzz.
  inline bool isZero(const Real& x)
  {
    return isZeroWithError(x,MathConsts::RealMin);
  }
	
  /// Function to check if a Real number is not zero or very close.
  /// @param x
  /// @return true if not equal to zero or not almost equal within the accepted error fuzz.
  inline bool isNotZero(const Real& x)
  {
    return isNotZeroWithError(x,MathConsts::RealMin);
  }
	
  /// Checks is real is positive.
  /// Kind of a sign function returning a bool.
  inline bool isPositive(const Real& value)
  {
    return (value < 0.0) ? false : true;
  }
	
  /// Checks is real is negative.
  /// Kind of a sign function returning a bool.
  inline bool isNegative(const Real& value)
  {
    return !isPositive(value);
  }
	
} // namespace MathChecks

////////////////////////////////////////////////////////////////////////////////

} // namespace Math
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_MathChecks_hpp
