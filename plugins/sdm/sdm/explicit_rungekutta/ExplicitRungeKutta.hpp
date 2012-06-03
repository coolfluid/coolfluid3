// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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
/// Standard Explicit Runge-Kutta integration using butcher tableau coefficients
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

  /// raises the event when iteration done
  void raise_iteration_done();

  virtual void link_fields();

protected:
  Handle<ButcherTableau> m_butcher;

private: // data

  /// Registers necessary for general runge kutta algorithm
  Handle<mesh::Field> m_solution_backup;
  std::vector< Handle<mesh::Field> > m_residuals;
};

////////////////////////////////////////////////////////////////////////////////

class sdm_explicit_rungekutta_API ExplicitRungeKutta : public ExplicitRungeKuttaBase {

public: // functions

  /// Contructor
  /// @param name of the component
  ExplicitRungeKutta ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "ExplicitRungeKutta"; }

private: // functions

  virtual void config_butcher_tableau();

};

////////////////////////////////////////////////////////////////////////////////

template <typename BUTCHER_TABLEAU>
class ExplicitRungeKuttaT : public ExplicitRungeKuttaBase {
public:

  static std::string type_name () { return BUTCHER_TABLEAU::name(); }

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
