//#ifndef StandardKEpsilon_H
//#define StandardKEpsilon_H

//#endif // StandardKEpsilon_H


// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_StandardKEpsilon_hpp
#define cf3_UFEM_StandardKEpsilon_hpp

#include <solver/Action.hpp>

#include "LibUFEM.hpp"
#include "KEpsilonBase.hpp"

namespace cf3 {

namespace UFEM {

/// solver for StandardKEpsilon turbulence model
class UFEM_API StandardKEpsilon : public KEpsilonBase
{
public: // functions

  /// Contructor
  /// @param name of the component
  StandardKEpsilon ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "StandardKEpsilon"; }

protected:
  virtual void do_set_expressions(LSSActionUnsteady& lss_action, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u);

private:
  Real m_kappa = 0.41;
  Real m_yplus = 11.06;
};

} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokes_hpp
