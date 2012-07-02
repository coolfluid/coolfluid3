// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_Checks_hpp
#define cf3_Math_Checks_hpp

#include "common/CF.hpp"

#include "math/BoostMath.hpp"
#include "math/Consts.hpp"
#include "math/FloatingPoint.hpp"

namespace cf3 {
namespace math {

////////////////////////////////////////////////////////////////////////////////////////////


/// @brief Static functions for checking Real numbers
/// @author Tiago Quintino, Willem Deconinck
namespace Checks {

/// Function to check if two Real numbers are equal.
/// @param x
/// @param y
/// @param fuzz
/// @return true if equal or almost equal within the accepted error fuzz.
inline bool is_equal_with_error(const Real& x, const Real& y, const Real& fuzz)
{
  // see Knuth section 4.2.2 pages 217-218
  return std::abs(x - y) <= fuzz * std::max(std::abs(x),std::abs(y));
}

/// Function to check if two Real numbers are equal.
/// @param x
/// @param y
/// @return true if equal or almost equal within the accepted error fuzz.
inline bool is_equal(const Real& x,  const Real& y)
{
  return is_equal_with_error(x,y,Consts::eps());
}

/// Function to check if two Real numbers are not equal.
/// @param x
/// @param y
/// @return true if not equal or almost unequal within the accepted error fuzz.
inline bool is_not_equal(const Real& x,  const Real& y)
{
  return !is_equal_with_error(x,y,Consts::eps());
}

/// Function to check if a Real number is zero or very close.
/// @param x
/// @return true if equal to zero or almost equal within the accepted error fuzz.
inline bool is_zero(const Real& x)
{
  return is_equal_with_error(x, 0.0, 100*Consts::eps());
}

/// Function to check if a Real number is not zero or very close.
/// @param x
/// @return true if not equal to zero or not almost equal within the accepted error fuzz.
inline bool is_not_zero(const Real& x)
{
  return !is_zero(x);
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

} // Checks

////////////////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3

#endif // cf3_Math_Checks_hpp
