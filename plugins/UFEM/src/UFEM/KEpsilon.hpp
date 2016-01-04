//#ifndef KEpsilon_H
//#define KEpsilon_H

//#endif // KEpsilon_H


// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_KEpsilon_hpp
#define cf3_UFEM_KEpsilon_hpp

#include <solver/Action.hpp>

#include "LibUFEM.hpp"
#include "SUPG.hpp"

#include "CrossWindDiffusion.hpp"

namespace cf3 {

namespace UFEM {

/// solver for KEpsilon turbulence model
class UFEM_API KEpsilon : public solver::Action
{
public: // functions

  /// Contructor
  /// @param name of the component
  KEpsilon ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "KEpsilon"; }

  virtual void execute();

private:
  void trigger_set_expression();
  virtual void on_regions_set();

  // SUPG helpers
  ComputeTau compute_tau;
  Real tau_su;

  // Parameter for the theta scheme
  Real m_theta = 0.5;
  Real m_sigma_k = 1.;
  Real m_sigma_epsilon = 1.3;
  Real m_c_epsilon_1 = 1.35;
  Real m_c_epsilon_2 = 1.8;
  Real m_c_mu = 0.09;
  Real m_minimal_viscosity_ratio = 0.1;
  Real m_l_max = 1.; // Maximum mixing length

  solver::actions::Proto::MakeSFOp<CrosswindDiffusion>::stored_type m_diff_data;
  solver::actions::Proto::MakeSFOp<CrosswindDiffusion>::reference_type diffusion_coeff;
};

} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokes_hpp
