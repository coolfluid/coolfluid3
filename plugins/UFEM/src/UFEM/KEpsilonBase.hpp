//#ifndef KEpsilonBase_H
//#define KEpsilonBase_H

//#endif // KEpsilonBase_H


// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_KEpsilonBase_hpp
#define cf3_UFEM_KEpsilonBase_hpp

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

/// solver for KEpsilonBase turbulence model
class UFEM_API KEpsilonBase : public solver::Action
{
public: // functions

  /// Contructor
  /// @param name of the component
  KEpsilonBase ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "KEpsilonBase"; }

  virtual void execute();

  virtual ~KEpsilonBase();

protected:
  virtual void do_set_expressions(LSSActionUnsteady& lss_action, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u) = 0;
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
  FieldVariable<1, ScalarField> epsilon;
  FieldVariable<3, ScalarField> nu_eff;
  FieldVariable<4, ScalarField> d;
  FieldVariable<5, ScalarField> yplus;

  Real m_nu_lam = 0;

  // SUPG helpers
  ComputeTau compute_tau;
  Real tau_su;

  // Parameter for the theta scheme
  Real m_theta = 0.5;

  // K-Epsilon model constants
  Real m_sigma_k = 1.;
  Real m_sigma_epsilon = 1.3;
  Real m_c_epsilon_1 = 1.35;
  Real m_c_epsilon_2 = 1.8;
  Real m_c_mu = 0.;
  Real m_minimal_viscosity_ratio = 1e-4;
  Real m_l_max = 1e6; // Maximum mixing length

  CrosswindDiffusion cw;
};

} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokes_hpp
