// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Physics_Consts_hpp
#define cf3_Physics_Consts_hpp

#include <limits>    // for std::numeric_limits

#include "common/CF.hpp"

#include "physics/LibPhysics.hpp"

namespace cf3 {
namespace physics {

////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Static functions for physical constants
///
/// @author Tiago Quintino
/// @author Willem Deconinck
namespace Consts
{
  /// Definition of the Boltzmann's constant [J/K]
  inline Real kB()  { return 1.3806503e-23; }
  /// Definition of the Unit charge [C]
  inline Real e()   { return 1.60217646e-19; }
  /// Definition of Avogadro's constant [1/mol]
  inline Real NA()  { return 6.02214199e23; }
  /// Definition of the Faraday's constant [C/mol]   ( = e() * NA() )
  inline Real F()   { return 96485.341352; }
  /// Definition of the ideal gas constant [J/mol K] ( = kb() * NA() )
  inline Real R()   { return 8.3144721451; }
  /// Definition of the Euler gamma
  inline Real gamma() { return 0.57721566; }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Physics
} // cf3

#endif // cf3_Physics_Consts_hpp
