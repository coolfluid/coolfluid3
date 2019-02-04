//#ifndef keAdjoint_H
//#define keAdjoint_H

//#endif // keAdjoint_H


// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_keAdjoint2_hpp
#define cf3_UFEM_keAdjoint2_hpp

#include <solver/Action.hpp>

#include "LibUFEMAdjoint.hpp"
#include "keAdjointbase.hpp"

namespace cf3 {

namespace UFEM {
namespace adjoint{

/// solver for keAdjoint turbulence model
class UFEM_API keAdjoint2 : public keAdjointbase
{
public: // functions

  /// Contructor
  /// @param name of the component
  keAdjoint2 ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "keAdjoint2"; }

protected:
  virtual void do_set_expressions(LSSActionUnsteady& lss_action);//, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u);
 Real m_c_tau = 6;
};
} // adjoint
} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokes_hpp
