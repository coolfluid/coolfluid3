// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_RungeKutta_hpp
#define cf3_sdm_RungeKutta_hpp

#include "sdm/IterativeSolver.hpp"

namespace cf3 {
namespace sdm {

/////////////////////////////////////////////////////////////////////////////////////

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
class sdm_API ExplicitRungeKutta : public IterativeSolver {

public: // functions

  /// Contructor
  /// @param name of the component
  ExplicitRungeKutta ( const std::string& name );

  /// Virtual destructor
  virtual ~ExplicitRungeKutta() {}

  /// Get the class name
  static std::string type_name () { return "ExplicitRungeKutta"; }

  /// execute the action
  virtual void execute ();

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

private: // functions

  /// raises the event when iteration done
  void raise_iteration_done();

  virtual void link_fields();

  void config_coefficients();

private: // data

  std::vector<Real> m_a;
  std::vector<Real> m_b;
  std::vector<Real> m_c;

  /// Registers necessary for general runge kutta algorithm
  Handle<mesh::Field> m_solution_backup;
  std::vector< Handle<mesh::Field> > m_residuals;
};

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3

#endif // cf3_sdm_RungeKutta_hpp
