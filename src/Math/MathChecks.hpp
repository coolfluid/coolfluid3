#ifndef CF_Math_MathChecks_hh
#define CF_Math_MathChecks_hh

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/NonInstantiable.hpp"

#include "Math/BoostHeaders.hpp"
#include "Math/MathConsts.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// Provides an a set of static functions for checking Real numbers.
/// This class is not instantiable.
/// @author Tiago Quintino
class MathChecks : public Common::NonInstantiable<MathChecks> {
public:

  /// Function to check if two Real numbers are equal.
  /// @param x
  /// @param y
  /// @return true if equal or almost equal within the accepted error fuzz.
  static bool isEqual(const Real& x,  const Real& y)
  {
    return isEqualWithError(x,y,Math::MathConsts::RealMin());
  }

  /// Function to check if two Real numbers are not equal.
  /// @param x
  /// @param y
  /// @return true if not equal or almost unequal within the accepted error fuzz.
  static bool isNotEqual(const Real& x,  const Real& y)
  {
    return isNotEqualWithError(x,y,Math::MathConsts::RealMin());
  }

  /// Function to check if two Real numbers are equal.
  /// @param x
  /// @param y
  /// @param fuzz
  /// @return true if equal or almost equal within the accepted error fuzz.
  static bool isEqualWithError(const Real& x, const Real& y, const Real& fuzz)
  {
    // see Knuth section 4.2.2 pages 217-218
    return std::abs(x - y) <= fuzz * std::abs(x);
  }

  /// Function to check if two Real numbers are not equal.
  /// @param x
  /// @param y
  /// @param fuzz
  /// @return true if not equal or almost unequal within the accepted error fuzz.
  static bool isNotEqualWithError(const Real& x, const Real& y, const Real& fuzz)
  {
    return !isEqualWithError(x,y,fuzz);
  }

  /// Function to check if a Real number is finite number. This means is not a NaN neither a INF
  /// @param x
  /// @return true if x is finite
  static bool isFinite(const Real& x)
  {
    return (boost::math::isfinite)(x);
  }

  /// Function to check if a Real number is either minus or plus INF
  /// @param x
  /// @return true if x is finite
  static bool isInf(const Real& x)
  {
    return (boost::math::isinf)(x);
  }

  /// Function to check if a Real number is a NaN (Not a Number)
  /// @param x
  /// @return true if x is a NaN
  static bool isNaN(const Real& x)
  {
    return (boost::math::isnan)(x);
  }

  /// Function to check if a Real number is zero or very close.
  /// @param x
  /// @return true if equal to zero or almost equal within the accepted error fuzz.
  static bool isZeroWithError(const Real& x, const Real& fuzz)
  {
    return std::abs(x) <= fuzz;
  }


  /// Function to check if a Real number is not zero or very close.
  /// @param x
  /// @return true if not equal to zero or not almost equal within the accepted error fuzz.
  static bool isNotZeroWithError(const Real& x, const Real& fuzz)
  {
    return !isZeroWithError(x,fuzz);
  }

  /// Function to check if a Real number is zero or very close.
  /// @param x
  /// @return true if equal to zero or almost equal within the accepted error fuzz.
  static bool isZero(const Real& x)
  {
    return isZeroWithError(x,Math::MathConsts::RealMin());
  }

  /// Function to check if a Real number is not zero or very close.
  /// @param x
  /// @return true if not equal to zero or not almost equal within the accepted error fuzz.
  static bool isNotZero(const Real& x)
  {
    return isNotZeroWithError(x,Math::MathConsts::RealMin());
  }

  /// Sign function returning a real
  static Real sign(const Real& value)
  {
    return (value < 0.0) ? -1.0 : 1.0;
  }

  /// Checks is real is positive.
  /// Kind of a sign function returning a bool.
  static bool isPositive(const Real& value)
  {
    return (value < 0.0) ? false : true;
  }

  /// Checks is real is negative.
  /// Kind of a sign function returning a bool.
  static bool isNegative(const Real& value)
  {
    return !isPositive(value);
  }

}; // end class MathChecks

////////////////////////////////////////////////////////////////////////////////

} // namespace Math
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_MathChecks_hh
