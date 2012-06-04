// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/explicit_rungekutta/Types.hpp
/// @author Willem Deconinck
/// @author Matteo Parsani
///
/// This file defines several Butcher tableaux,
/// and corresonding Explicit Runge-Kutta methods

#include "sdm/explicit_rungekutta/ExplicitRungeKutta.hpp"

namespace cf3 {
namespace sdm {
namespace explicit_rungekutta {

////////////////////////////////////////////////////////////////////////////////

// Forward declarations of Butcher tableaux.
// The declarations are below

/// @brief Preconfigured Butcher tableaux
///
/// The acronym RK is used to indicate a Runge-Kutta method
/// The acronym RKF is used to indicate a Runge-Kutta-Fehlberg method
/// The acronym SSP is used to indicate a strong stability preserving method 
/// RK is always followed by two integer numbers: number of stages and order
/// of the method
namespace butcher_tableau
{
  struct ForwardEuler;
  struct Heun2;
  struct MidPoint;
  struct ClassicRK33;
  struct Heun3;
  struct ClassicRK44;
  struct SSPRK54;
  struct RK65;
  struct RKF65;
}

////////////////////////////////////////////////////////////////////////////////

// Preconfigured Explicit Runge-Kutta methods, templated by a Butcher tableau
// Their type-name = "cf3.sdm.explicit_rungekutta.<butcher_tableau>", with
// <butcher_tableau> the name of the Butcher Tableau

/// @brief forward Euler time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::ForwardEuler> ForwardEuler;

/// @brief 2-stage 2nd-order Heun time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::Heun2>        Heun2;

/// @brief MidPoint time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::MidPoint>     MidPoint;

/// @brief Classic RK33 time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::ClassicRK33>  ClassicRK33;

/// @brief 3-stage 3rd-order Heun time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::Heun3>        Heun3;

/// @brief Classic RK44 time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::ClassicRK44>  ClassicRK44;

/// @brief 5-stage 4th-order  SSPRK time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::SSPRK54>      SSPRK54;

/// @brief 6-stage 5th-order RK time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::RK65>         RK65;

/// @brief 6-stage 5th-order RKF time integration method
typedef ExplicitRungeKuttaT<butcher_tableau::RKF65>        RKF65;

////////////////////////////////////////////////////////////////////////////////

// Declarations of Butcher tableau's here:

namespace butcher_tableau {




/// 1st-order methods
/////////////////////

/// @brief forward Euler Butcher tableau coefficients
///
/// This method is also known as explicit Euler method
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




/// 2nd-order methods
/////////////////////

/// @brief Heun2 Butcher tableau coefficients
/// 
/// 2-stage 2nd-order Heun method
/// @ref K. Heun: Neue methode zur approximativen integration der differentialgleichungen einer unabhangigen variable
///      Z. Angew. Math. Phys., 45 (1900), 23–38
/// @verbatim
/// 0   |
/// 2/3 | 2/3
/// --------------
///     | 1/3  3/4
/// @endverbatim
struct Heun2 : ButcherTableau::Coefficients
{
  static std::string name() { return "Heun2"; }
  Heun2();
};




/// @brief mid-point Butcher tableau coefficients
/// @verbatim
/// 0   |
/// 1/2 | 1/2
/// --------------
///     | 0   1
/// @endverbatim
struct MidPoint : ButcherTableau::Coefficients
{
  static std::string name() { return "MidPoint"; }
  MidPoint();
};



/// 3rd-order methods
/////////////////////

/// @brief classic RK33 Butcher tableau coefficients
/// 
/// 3-stage 3rd-order Kutta's method 
/// @ref W. Kutta: Beitrag zur naherungsweisen integration totaler differentialgleichungen
///      Z. Angew. Math. Phys., 46 (1901), 435–453
/// @verbatim
/// 0   |
/// 1/2 | 1/2
/// 1   | -1    2
/// -------------------
///     | 1/6  2/3  1/6
/// @endverbatim
struct ClassicRK33 : ButcherTableau::Coefficients
{
  static std::string name() { return "ClassicRK33"; }
  ClassicRK33();
};




/// @brief Heun3 Butcher tableau coefficients
///
/// 3-stage 23rd-order Heun method
/// @ref K. Heun: Neue methode zur approximativen integration der differentialgleichungen einer unabhangigen variable
///      Z. Angew. Math. Phys., 45 (1900), 23–38
/// @verbatim
/// 0   |
/// 1/3 | 1/3  
/// 2/3 | 0    2/3
/// -------------------
///     | 1/4  0    3/4
/// @endverbatim
struct Heun3 : ButcherTableau::Coefficients
{
  static std::string name() { return "Heun3"; }
  Heun3();
};




/// 4th-order methods
/////////////////////

/// @brief classic RK44 Butcher tableau coefficients
///
/// 4-stage 4th-order Kutta's method 
/// @ref W. Kutta: Beitrag zur naherungsweisen Integration totaler differentialgleichungen
///      Z. Angew. Math. Phys., 46 (1901), 435–453
/// @verbatim
/// 0   |
/// 1/2 | 1/2
/// 1/2 | 0    1/2
/// 1   | 0    0    1
/// ------------------------
///     | 1/6  1/3  1/3  1/6
/// @endverbatim
struct ClassicRK44 : ButcherTableau::Coefficients
{
  static std::string name() { return "ClassicRK44"; }
  ClassicRK44();
};




/// @brief  SSPRK54 Butcher tableau coefficients
///
/// Optimal 5-stage 4th-order SSPRK method
/// @ref R. J. Spiteri and S. J. Ruuth: A new class of optimal high order strong stability preserving time discretization methods
///      SIAM Journal of Numerical Analysis, 40 (2002), 469-491
///      doi:10.1137/S0036142901389025
/// @verbatim
/// 0                |
/// 0.39175222700392 | 0.39175222700392
/// 0.58607968896780 | 0.21766909633821  0.36841059262959                
/// 0.47454236302687 | 0.08269208670950  0.13995850206999  0.25189177424738
/// 0.93501063100924 | 0.06796628370320  0.11503469844438  0.20703489864929  0.54497475021237
/// -----------------------------------------------------------------------------------------------------------
///                  | 0.14681187618661  0.24848290924556  0.10425883036650  0.27443890091960  0.22600748319395
/// @endverbatim
struct SSPRK54 : ButcherTableau::Coefficients
{
  static std::string name() { return "SSPRK54"; }
  SSPRK54();
};




/// 5th-order methods
/////////////////////

/// @brief RK65 Butcher tableau coefficients
///
/// A 6-stage 5th-order RK method
/// @ref J. C. Butcher: Numerical methods for ordinary differential equations
//       John Wiley & Sons, Ltd
/// @verbatim
/// 0   |
/// 1/4 | 1/4
/// 1/4 | 1/8   1/8
/// 1/2 | 0     0     1/2
/// 3/4 | 3/16  -3/8  3/8    9/16
/// 1   | -3/7  8/7   6/7    -12/7  8/7 
/// ---------------------------------------------
///     | 7/90  0     32/90  12/90  32/90  7/90
/// @endverbatim
struct RK65 : ButcherTableau::Coefficients
{
  static std::string name() { return "RK65"; }
  RK65();
};




/// @brief RKF65 Butcher tableau coefficients
///
/// 6-stage 5th-order RKF method extracted form the RKF6(4)5 pair
/// @ref E. Fehlberg. Low-order classical Runge-Kutta formulas with stepsize control and their application to some heat transfer
///      Technical report, NASA TR R-315, National Aeronautics and Space Administration, Marshall Space Flight Center, Marshall, AL, 1969.
/// @verbatim
/// 0     |
/// 1/4   | 1/4
/// 3/8   | 3/32       9/32
/// 12/13 | 1932/2197  -7200/2197  7296/2197
/// 1     | 439/216    -8          3680/513    -845/4104
/// 1/2   | -8/27      2           -3544/2565  1859/4104   -11/40 
/// ----------------------------------------------------------------
///       | 16/135,    0           6656/12825  28561/56430 -9/50  2/55
/// @endverbatim
struct RKF65 : ButcherTableau::Coefficients
{
  static std::string name() { return "RKF65"; }
  RKF65();
};





} // butcher_tableau

////////////////////////////////////////////////////////////////////////////////

} // explicit_rungekutta
} // sdm
} // cf3
