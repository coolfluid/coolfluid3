// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_Consts_hpp
#define CF_Math_Consts_hpp

#include <limits>    // for std::numeric_limits

#include "Common/CF.hpp"

#include "Math/LibMath.hpp"

namespace CF {
namespace Math {

////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Static functions for mathematical constants
///
/// @author Tiago Quintino
/// @author Willem Deconinck
namespace Consts
{
  /// Returns the maximum number representable with the chosen precision
  inline Real int_max() { return std::numeric_limits<int>::max(); }
  /// Definition of the minimum number representable with the chosen precision.
  inline Real int_min() { return std::numeric_limits<int>::min(); }
  /// Returns the maximum number representable with the chosen precision
  inline Uint uint_max() { return std::numeric_limits<Uint>::max(); }
  /// Definition of the minimum number representable with the chosen precision.
  inline Uint uint_min() { return std::numeric_limits<Uint>::min(); }
  /// Returns the maximum number representable with the chosen precision
  inline Real real_max() { return std::numeric_limits<Real>::max(); }
  /// Definition of the minimum number representable with the chosen precision.
  inline Real real_min() { return std::numeric_limits<Real>::min(); }
  /// Definition of the maximum difference recognazible between two numbers with
  /// the chosen precision. Usefull for comparisons to zero  with real numbers:
  /// @code std::abs(x) > Math::Consts::eps()  @endcode
  inline Real eps() { return std::numeric_limits<Real>::epsilon(); }
  /// Definition of Infinity
  inline Real inf() { return std::numeric_limits<Real>::infinity(); }
  /// Definition of the Pi constant.
  inline Real pi() { return M_PI; }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Math
} // CF

#endif // CF_Math_Consts_hpp
