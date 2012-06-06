// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/explicit_rungekutta/Types.hpp
/// @author Willem Deconinck
///
/// This file defines several Butcher-tableau's,
/// and corresonding Explicit Runge-Kutta methods

#include "sdm/explicit_rungekutta/ExplicitRungeKutta.hpp"

namespace cf3 {
namespace sdm {
namespace explicit_rungekutta {

////////////////////////////////////////////////////////////////////////////////

// Forward declarations of Butcher tableau's.
// The declarations are below

/// @brief Preconfigured butcher tableau's
namespace butcher_tableau
{
  struct ForwardEuler;
  struct Heun;
  struct RK3;
  struct ClassicRK4;
}

////////////////////////////////////////////////////////////////////////////////

// Preconfigured Explicit Runge-Kutta methods, templated by a Butcher tableau
// Their type-name = "cf3.sdm.explicit_rungekutta.<butcher_tableau>", with
// <butcher_tableau> the name of the Butcher Tableau

/// @brief Forward Euler time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::ForwardEuler> ForwardEuler;

/// @brief Heun time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::Heun>         Heun;

/// @brief RK3 time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::RK3>          RK3;

/// @brief Classic RK4 time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::ClassicRK4>   ClassicRK4;

////////////////////////////////////////////////////////////////////////////////

// Declarations of Butcher tableau's here:

namespace butcher_tableau {




/// @brief Forward Euler Butcher tableau coefficients
/// @verbatim
/// 0 |
/// ------
///   | 1
/// @endverbatim
struct ForwardEuler : ButcherTableau::Coefficients
{
  static std::string name() { return "ForwardEuler"; }
  ForwardEuler();
};





/// @brief Heun Butcher tableau coefficients
/// @verbatim
/// 0   |
/// 2/3 | 2/3
/// --------------
///     | 1/3  3/4
/// @endverbatim
struct Heun : ButcherTableau::Coefficients
{
  static std::string name() { return "Heun"; }
  Heun();
};




/// @brief RK3 Butcher tableau coefficients
/// @verbatim
/// 0   |
/// 1/2 | 1/2
/// 1   | -1    2
/// -------------------
///     | 1/6  2/3  1/6
/// @endverbatim
struct RK3 : ButcherTableau::Coefficients
{
  static std::string name() { return "RK3"; }
  RK3();
};





/// @brief Classic RK4 Butcher tableau coefficients
/// @verbatim
/// 0   |
/// 1/2 | 1/2
/// 1/2 | 0    1/2
/// 1   | 0    0    1
/// ------------------------
///     | 1/6  1/3  1/3  1/6
/// @endverbatim
struct ClassicRK4 : ButcherTableau::Coefficients
{
  static std::string name() { return "ClassicRK4"; }
  ClassicRK4();
};





} // butcher_tableau

////////////////////////////////////////////////////////////////////////////////

} // explicit_rungekutta
} // sdm
} // cf3
