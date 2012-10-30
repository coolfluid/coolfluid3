// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_StokesSteady_hpp
#define cf3_UFEM_StokesSteady_hpp

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "LSSAction.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

namespace cf3 {

namespace UFEM {

/// Special case of steady FEM problems: only one LSS solve is needed to get the solution of the problem
/// Useful for i.e. linear heat conduction
class UFEM_API StokesSteady : public LSSAction
{
public: // functions

  /// Contructor
  /// @param name of the component
  StokesSteady ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "StokesSteady"; }

private:
  virtual void on_initial_conditions_set(InitialConditions& initial_conditions);
};

} // UFEM
} // cf3


#endif // cf3_UFEM_StokesSteady_hpp
