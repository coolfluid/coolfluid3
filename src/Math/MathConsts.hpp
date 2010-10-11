// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Math_MathConsts_hpp
#define CF_Math_MathConsts_hpp

////////////////////////////////////////////////////////////////////////////////

#include <limits>    // for std::numeric_limits

#include "Common/CF.hpp"
#include "Common/NonInstantiable.hpp"

#include "Math/LibMath.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {

////////////////////////////////////////////////////////////////////////////////

/// Provides an a set of static functions for mathematical constants
/// @author Tiago Quintino
class MathConsts : public Common::NonInstantiable<MathConsts> {
public:
  /// Returns the maximum number representable with the chosen precision
  static Real intMax () { return std::numeric_limits<int>::max(); }
  /// Definition of the minimum number representable with the chosen precision.
  static Real intMin () { return std::numeric_limits<int>::min(); }
  /// Returns the maximum number representable with the chosen precision
  static Uint UintMax () { return std::numeric_limits<Uint>::max(); }
  /// Definition of the minimum number representable with the chosen precision.
  static Uint UintMin () { return std::numeric_limits<Uint>::min(); }
  /// Returns the maximum number representable with the chosen precision
  static Real RealMax () { return std::numeric_limits<Real>::max(); }
  /// Definition of the minimum number representable with the chosen precision.
  static Real RealMin () { return std::numeric_limits<Real>::min(); }
  /// Definition of the maximum difference recognazible between two numbers with
  /// the chosen precision. Usefull for comparisons to zero  with real numbers:
  /// @code std::abs(x) > Math::MathConsts::RealEps()  @endcode
  static Real RealEps () { return std::numeric_limits<Real>::epsilon(); }
  /// Definition of Infinity
  static Real RealInf () { return std::numeric_limits<Real>::infinity(); }
  /// Definition of the Pi constant.
  static Real RealPi  () { return M_PI; }
//  /// Definition of the imaginary constant i = sqrt(-1)
//  static Complex complexI () { return Complex(0.0,1.0); }
};

////////////////////////////////////////////////////////////////////////////////

} // Math
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Math_MathConsts_hpp
