// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_ExplicitRungeKuttaLowStorage2_hpp
#define cf3_sdm_ExplicitRungeKuttaLowStorage2_hpp

#include "sdm/IterativeSolver.hpp"

namespace cf3 {
namespace sdm {

/////////////////////////////////////////////////////////////////////////////////////

/// Runge-Kutta low storage integration method using only 2 registers
/// @ref David I. Ketcheson: Runge-Kutta methods with minimum storage implementations
///      Journal of Computational Physics 229 (2010) 1763–1773
///      doi:10.1016/j.jcp.2009.11.006
/// The order is not necessarily the same as the number of stages "m"
/// The order depends on the coefficients alpha and beta
/// Algorithm 2S* with m = number of stages (not necessarily same as order)
/// @code
/// S1 := U(t=n)   S2 := U(t=n)
/// for i = 2:m+1 do
///    S1 := (1-alpha(i,1))*S1 + alpha(i,1)*S2 + beta(i,i-1)*dt*F(S1)
/// end
/// U(t=n+1) = S1
/// @endcode
/// @author Willem Deconinck
class sdm_API ExplicitRungeKuttaLowStorage2 : public IterativeSolver {

public: // functions

  /// Contructor
  /// @param name of the component
  ExplicitRungeKuttaLowStorage2 ( const std::string& name );

  /// Virtual destructor
  virtual ~ExplicitRungeKuttaLowStorage2() {}

  /// Get the class name
  static std::string type_name () { return "ExplicitRungeKuttaLowStorage2"; }

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

  std::vector<Real> m_alpha;
  std::vector<Real> m_beta;
  std::vector<Real> m_gamma;

  /// Second register necessary for low-storage runge kutta algorithm  2S*
  Handle<mesh::Field> m_solution_backup;
};

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3

#endif // cf3_sdm_ExplicitRungeKuttaLowStorage2_hpp
