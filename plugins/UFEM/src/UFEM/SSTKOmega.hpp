//#ifndef SSTKOmega_H
//#define SSTKOmega_H

//#endif // SSTKOmega_H


// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SSTKOmega_hpp
#define cf3_UFEM_SSTKOmega_hpp

#include "common/CF.hpp"

#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "solver/Action.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

#include "LibUFEM.hpp"
#include "SUPG.hpp"

#include "LSSActionUnsteady.hpp"

#include "CrossWindDiffusion.hpp"

namespace cf3 {

namespace UFEM {

/// solver for SSTKOmega turbulence model
class UFEM_API SSTKOmega : public solver::Action
{
public: // functions

  /// Contructor
  /// @param name of the component
  SSTKOmega ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "SSTKOmega"; }

  virtual void execute();

  virtual ~SSTKOmega();

protected:
  virtual void do_set_expressions(LSSActionUnsteady& lss_action, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u);
  void trigger_set_expression();
  virtual void on_regions_set();

  boost::mpl::vector5<
    mesh::LagrangeP1::Quad2D,
    mesh::LagrangeP1::Triag2D,
    mesh::LagrangeP1::Hexa3D,
    mesh::LagrangeP1::Tetra3D,
    mesh::LagrangeP1::Prism3D
  > allowed_elements;

  FieldVariable<0, ScalarField> k;
  FieldVariable<1, ScalarField> omega;
  FieldVariable<3, ScalarField> nu_eff;
  FieldVariable<4, ScalarField> d;
  FieldVariable<5, ScalarField> yplus;

  Real m_nu_lam = 0;

  // SUPG helpers
  ComputeTau compute_tau;
  Real tau_su;

  // Parameter for the theta scheme
  Real m_theta = 0.5;

  // Physics constants
  Real m_rho = 1.2;

  // K-Omega model constants
  Real m_beta_s = 0.09;
  Real m_sigma_k1 = 0.85;
  Real m_sigma_k2 = 1.;
  Real m_sigma_omega1 = 0.5;
  Real m_sigma_omega2 = 0.856;
  Real m_beta_1 = 0.075;
  Real m_beta_2 = 0.0282;
  Real m_gamma_1;
  Real m_gamma_2;

  CrosswindDiffusion cw;
};

} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokes_hpp
