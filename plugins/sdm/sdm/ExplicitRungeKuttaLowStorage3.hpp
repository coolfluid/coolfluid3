// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_ExplicitRungeKuttaLowStorage3_hpp
#define cf3_sdm_ExplicitRungeKuttaLowStorage3_hpp

#include "sdm/IterativeSolver.hpp"

namespace cf3 {
namespace sdm {

/////////////////////////////////////////////////////////////////////////////////////

/// Runge-Kutta low storage integration method using only 3 registers
/// @ref David I. Ketcheson: Runge-Kutta methods with minimum storage implementations
///      Journal of Computational Physics 229 (2010) 1763–1773
///      doi:10.1016/j.jcp.2009.11.006
/// The order is not necessarily the same as the number of stages "m"
/// The order depends on the coefficients alpha and beta
/// Algorithm 3S* with m = number of stages (not necessarily same as order)
/// @code
/// // Use convention indexes start at 1
/// S1 := U(t=n)   S2 := 0   S3 := U(t=n)
/// for i = 2:m+1 do
///     S2 := S2 + delta(i-1)*S1
///     S1 := gamma(i,1)*S1 + gamma(i,2)*S2 + gamma(i,3)*S3 + beta(i,i-1)*dt*F(S1)
/// end
/// U(t=n+1) = S1
/// // for error_estimate, use:
///     S2 := 1/sum(delta) * (S2 + delta(m+1)*S1 + delta(m+2)*S3

/// @endcode
/// @author Willem Deconinck
class sdm_API ExplicitRungeKuttaLowStorage3 : public IterativeSolver {

public: // functions

  /// Contructor
  /// @param name of the component
  ExplicitRungeKuttaLowStorage3 ( const std::string& name );

  /// Virtual destructor
  virtual ~ExplicitRungeKuttaLowStorage3() {}

  /// Get the class name
  static std::string type_name () { return "ExplicitRungeKuttaLowStorage3"; }

  /// execute the action
  virtual void execute ();

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

private: // functions

  /// raises the event when iteration done
  void raise_iteration_done();

  void config_nb_stages();

  virtual void link_fields();

private: // data

  /// Second register necessary for low-storage runge kutta algorithm 3S*
  Handle<mesh::Field> m_S2;
  /// Third register necessary for low-storage runge kutta algorithm  3S*
  Handle<mesh::Field> m_solution_backup; // ( = S3 in algorithm )
};

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3

#endif // cf3_sdm_ExplicitRungeKuttaLowStorage3_hpp
