//#ifndef ChienKEpsilon_H
//#define ChienKEpsilon_H

//#endif // ChienKEpsilon_H


// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_ChienKEpsilon_hpp
#define cf3_UFEM_ChienKEpsilon_hpp

#include <solver/Action.hpp>

#include "LibUFEM.hpp"
#include "SUPG.hpp"

#include "KEpsilonBase.hpp"

namespace cf3 {

namespace UFEM {

/// solver for ChienKEpsilon turbulence model
class UFEM_API ChienKEpsilon : public KEpsilonBase
{
public: // functions

  /// Contructor
  /// @param name of the component
  ChienKEpsilon ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "ChienKEpsilon"; }

protected:
  virtual void do_set_expressions(LSSActionUnsteady& lss_action, solver::actions::Proto::ProtoAction& update_nut, FieldVariable<2, VectorField>& u);
};

} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokes_hpp
