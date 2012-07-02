// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/explicit_rungekutta/ExplicitRungeKutta.hpp
/// @author Willem Deconinck
/// @author Matteo Parsani
///
/// This file includes the ExplicitRungeKuttaBase component class,
/// defining the Explicit Runge Kutta scheme,
/// as well as the ExplicitRungeKutta component class, and a templated
/// ExplicitRungeKuttaT<BUTCHERTABLEAU> component class , deriving from
/// ExplicitRungeKuttaBase.
///
/// The ExplicitRungeKutta component is configurable
/// with a "order" option which will automatically configure some
/// default Butcher tableau's.
///
/// The ExplicitRungeKuttaT<BUTCHERTABLEAU> component is a compile-time
/// configured ExplicitRungeKuttaBase component class, by a templated
/// BUTCHERTABLEAU struct. These structs are defined in
/// the file sdm/explicit_rungekutta/Types.hpp

#ifndef cf3_sdm_explicit_rungekutta_RungeKuttaBase_hpp
#define cf3_sdm_explicit_rungekutta_RungeKuttaBase_hpp

#include "sdm/IterativeSolver.hpp"

#include "sdm/explicit_rungekutta/ButcherTableau.hpp"

namespace cf3 {
namespace sdm {
namespace explicit_rungekutta {

////////////////////////////////////////////////////////////////////////////////

/// @brief Runge-Kutta integration method
///
/// Standard Explicit Runge-Kutta integration using Butcher tableau coefficients
/// @verbatim
/// 0  |
/// c2 | a21
/// c3 | a31  a32
///  : |  :       `-.
/// cs | as1  as2  ..  as,s-1
/// ------------------------------
///    | b1   b2   ..   bs-1    bs
/// @endverbatim
/// @code
/// t = t0
/// U0 = U
/// for i = 1 : s do
///     t := t0 + c(i)*dt
///     U := U0;
///     for j = 1:i-1 do
///         U := U + dt * a(i,j) * R(j);
///     end
///     R(i) := F( U );
/// end
/// U = U0
/// for i = 1 : s do
///     U := U + dt * b(i) * R(i)
/// end
/// t = t0
/// @endcode
///
/// Configuration of the coefficients a is a complete matrix, even though the matrix is lower-triangular
/// a:array[real]=a11,a12,...,a1s, a21,a22,...,a2s, ... , as1,as2,...,ass
/// b:array[real]=b1,b2,...,bs
/// c:array[real]=c1,c2,...,cs
///
/// @author Willem Deconinck
class sdm_explicit_rungekutta_API ExplicitRungeKuttaBase : public IterativeSolver {

public: // functions

  /// Contructor
  /// @param name of the component
  ExplicitRungeKuttaBase ( const std::string& name );

  /// Virtual destructor
  virtual ~ExplicitRungeKuttaBase() {}

  /// Get the class name
  static std::string type_name () { return "ExplicitRungeKuttaBase"; }

  /// execute the action
  virtual void execute ();

private: // functions

  virtual void link_fields();

protected:
  Handle<ButcherTableau> m_butcher;

private: // data

  // Registers necessary for general runge kutta algorithm
  Handle<mesh::Field> m_solution_backup;           ///< U0
  std::vector< Handle<mesh::Field> > m_residuals;  ///< R(i)
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Explicit Runge Kutta integration method
///
/// Configuring the option "order" automatically preconfigures some default
/// Butcher tableaux:
/// - order = 1 --> forward Euler (ForwardEuler)
/// - order = 2 --> 2-stage 2nd-order Heun method (Heun2)
/// - order = 3 --> classic 3-stage 3rd-order Runge-Kutta method (ClassicRK33)
/// - order = 4 --> classic 4-stage 4th-order Runge-Kutta method (ClassicRK44)
/// - order = 5 --> 6-stage 5th-order Runge-Kutta-Fehlberg method (from the Fehlberg pair) (RKF65)
/// If nothing is configured, ClassicRK44 is assumed.
/// @author Willem Deconinck
class sdm_explicit_rungekutta_API ExplicitRungeKutta : public ExplicitRungeKuttaBase {

public: // functions

  /// @brief Contructor
  /// @param name of the component
  ExplicitRungeKutta ( const std::string& name );

  /// @brief Get the class name
  static std::string type_name () { return "ExplicitRungeKutta"; }

private: // functions

  /// @brief Configure the butcher tableau when the option "order" is configured
  virtual void config_butcher_tableau();

};

////////////////////////////////////////////////////////////////////////////////

/// @brief Runge Kutta time integration method, templated by a butcher tableau
///
/// The typename of the component will be:
/// "cf3.sdm.explicit_rungekutta.<name>"  with <name> to be replaced by
/// the name of the Butcher tableau. For a list of Butcher tableaux, check
/// sdm/explicit_rungekutta/Types.hpp
/// @author Willem Deconinck
template <typename BUTCHER_TABLEAU>
class ExplicitRungeKuttaT : public ExplicitRungeKuttaBase {
public:

  /// @brief Type name
  static std::string type_name () { return BUTCHER_TABLEAU::name(); }

  /// @brief Constructor
  ExplicitRungeKuttaT(const std::string& name) : ExplicitRungeKuttaBase(name)
  {
    m_butcher = create_component<ButcherTableau>("butcher_tableau");
    m_butcher->set( BUTCHER_TABLEAU() );
  }
};

////////////////////////////////////////////////////////////////////////////////

} // explicit_rungekutta
} // sdm
} // cf3

#endif // cf3_sdm_explicit_rungekutta-ExplicitRungeKuttaBase_hpp
