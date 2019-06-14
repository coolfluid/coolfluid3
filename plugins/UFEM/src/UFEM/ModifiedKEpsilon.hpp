//#ifndef ModifiedKEpsilon_H
//#define ModifiedKEpsilon_H

//#endif // ModifiedKEpsilon_H


// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// Modified k-epsilon turbulence model for the actuator disc.
// Proposed by Amina El Kasmi and Christian Masson
// El Kasmi, Amina & Masson, Christian. (2008). An extended model for turbulent flow through horizontal-axis wind turbines. 
// Journal of Wind Engineering and Industrial Aerodynamics. 96. 103-122. 10.1016/j.jweia.2007.03.007. 


#ifndef cf3_UFEM_ModifiedKEpsilon_hpp
#define cf3_UFEM_ModifiedKEpsilon_hpp

#include <solver/Action.hpp>

#include "LibUFEM.hpp"
#include "KEpsilonBase.hpp"

namespace cf3 {

namespace UFEM {

/// solver for ModifiedKEpsilon turbulence model
class UFEM_API ModifiedKEpsilon : public KEpsilonBase
{
public: // functions

  /// Contructor
  /// @param name of the component
  ModifiedKEpsilon ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "ModifiedKEpsilon"; }

protected:
  FieldVariable<0, ScalarField> Ct;
  FieldVariable<1, ScalarField> density_ratio;
  virtual void do_set_expressions(LSSActionUnsteady& lss_action, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u);
  Real m_beta_d = 1.0;
  Real m_beta_0 = 0.1;
  Real m_c_epsilon_4 = 0.37;
  Real m_th = 10.0;
  Real m_ren = 0;
  Real m_Ct = 0.1;
  Real m_u_infty = 15.0;

};

} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokes_hpp
