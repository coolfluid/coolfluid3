// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_MathChecks_hpp
#define CF_Math_MathChecks_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"

#include "Math/BoostMath.hpp"
#include "Math/MathConsts.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {
	
////////////////////////////////////////////////////////////////////////////////

/// @brief Static functions for checking Real numbers
/// @author Tiago Quintino, Willem Deconinck
namespace MathChecks
{
  /// Function to check if two Real numbers are equal.
  /// @param x
  /// @param y
  /// @return true if equal or almost equal within the accepted error fuzz.
  bool is_equal(const Real& x,  const Real& y);

  /// Function to check if two Real numbers are not equal.
  /// @param x
  /// @param y
  /// @return true if not equal or almost unequal within the accepted error fuzz.
  bool is_not_equal(const Real& x,  const Real& y);

  /// Function to check if two Real numbers are equal.
  /// @param x
  /// @param y
  /// @param fuzz
  /// @return true if equal or almost equal within the accepted error fuzz.
  bool is_equal_with_error(const Real& x, const Real& y, const Real& fuzz);
	
  /// Function to check if two Real numbers are not equal.
  /// @param x
  /// @param y
  /// @param fuzz
  /// @return true if not equal or almost unequal within the accepted error fuzz.
  bool is_not_equal_with_error(const Real& x, const Real& y, const Real& fuzz);

  /// Function to check if a Real number is finite number. This means is not a NaN neither a INF
  /// @param x
  /// @return true if x is finite
  bool is_finite(const Real& x);

  /// Function to check if a Real number is either minus or plus INF
  /// @param x
  /// @return true if x is finite
  bool is_inf(const Real& x);

  /// Function to check if a Real number is a NaN (Not a Number)
  /// @param x
  /// @return true if x is a NaN
  bool is_nan(const Real& x);

  /// Function to check if a Real number is zero or very close.
  /// @param x
  /// @return true if equal to zero or almost equal within the accepted error fuzz.
  bool is_zero_with_error(const Real& x, const Real& fuzz);

  /// Function to check if a Real number is not zero or very close.
  /// @param x
  /// @return true if not equal to zero or not almost equal within the accepted error fuzz.
  bool is_not_zero_with_error(const Real& x, const Real& fuzz);

  /// Function to check if a Real number is zero or very close.
  /// @param x
  /// @return true if equal to zero or almost equal within the accepted error fuzz.
  bool is_zero(const Real& x);

  /// Function to check if a Real number is not zero or very close.
  /// @param x
  /// @return true if not equal to zero or not almost equal within the accepted error fuzz.
  bool is_not_zero(const Real& x);

  /// Checks is real is positive.
  /// Kind of a sign function returning a bool.
  bool is_pos(const Real& value);

  /// Checks is real is negative.
  /// Kind of a sign function returning a bool.
  bool is_neg(const Real& value);

	
	////////////////////////////////////////////////////////////////////////////////

		
	/// Function to check if two Real numbers are equal.
  /// @param x
  /// @param y
  /// @return true if equal or almost equal within the accepted error fuzz.
  inline bool is_equal(const Real& x,  const Real& y)
  {
    return is_equal_with_error(x,y,MathConsts::Real_min());
  }
	
  /// Function to check if two Real numbers are not equal.
  /// @param x
  /// @param y
  /// @return true if not equal or almost unequal within the accepted error fuzz.
  inline bool is_not_equal(const Real& x,  const Real& y)
  {
    return is_not_equal_with_error(x,y,MathConsts::Real_min());
  }
	
  /// Function to check if two Real numbers are equal.
  /// @param x
  /// @param y
  /// @param fuzz
  /// @return true if equal or almost equal within the accepted error fuzz.
  inline bool is_equal_with_error(const Real& x, const Real& y, const Real& fuzz)
  {
    // see Knuth section 4.2.2 pages 217-218
    return std::abs(x - y) <= fuzz * std::abs(x);
  }
	
  /// Function to check if two Real numbers are not equal.
  /// @param x
  /// @param y
  /// @param fuzz
  /// @return true if not equal or almost unequal within the accepted error fuzz.
  inline bool is_not_equal_with_error(const Real& x, const Real& y, const Real& fuzz)
  {
    return !is_equal_with_error(x,y,fuzz);
  }
	
  /// Function to check if a Real number is finite number. This means is not a NaN neither a INF
  /// @param x
  /// @return true if x is finite
  inline bool is_finite(const Real& x)
  {
    return (boost::math::isfinite)(x);
  }
	
  /// Function to check if a Real number is either minus or plus INF
  /// @param x
  /// @return true if x is finite
  inline bool is_inf(const Real& x)
  {
    return (boost::math::isinf)(x);
  }
	
  /// Function to check if a Real number is a NaN (Not a Number)
  /// @param x
  /// @return true if x is a NaN
  inline bool is_nan(const Real& x)
  {
    return (boost::math::isnan)(x);
  }
	
  /// Function to check if a Real number is zero or very close.
  /// @param x
  /// @return true if equal to zero or almost equal within the accepted error fuzz.
  inline bool is_zero_with_error(const Real& x, const Real& fuzz)
  {
    return std::abs(x) <= fuzz;
  }
	
	
  /// Function to check if a Real number is not zero or very close.
  /// @param x
  /// @return true if not equal to zero or not almost equal within the accepted error fuzz.
  inline bool is_not_zero_with_error(const Real& x, const Real& fuzz)
  {
    return !is_zero_with_error(x,fuzz);
  }
	
  /// Function to check if a Real number is zero or very close.
  /// @param x
  /// @return true if equal to zero or almost equal within the accepted error fuzz.
  inline bool is_zero(const Real& x)
  {
    return is_zero_with_error(x,MathConsts::Real_min());
  }
	
  /// Function to check if a Real number is not zero or very close.
  /// @param x
  /// @return true if not equal to zero or not almost equal within the accepted error fuzz.
  inline bool is_not_zero(const Real& x)
  {
    return is_not_zero_with_error(x,MathConsts::Real_min());
  }
	
  /// Checks is real is positive.
  /// Kind of a sign function returning a bool.
  inline bool is_pos(const Real& value)
  {
    return (value < 0.0) ? false : true;
  }
	
  /// Checks is real is negative.
  /// Kind of a sign function returning a bool.
  inline bool is_neg(const Real& value)
  {
    return !is_pos(value);
  }
	
} // MathChecks

////////////////////////////////////////////////////////////////////////////////

} // Math
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_MathChecks_hpp
