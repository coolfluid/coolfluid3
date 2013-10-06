// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesManual_hpp
#define cf3_UFEM_NavierStokesManual_hpp

#include <boost/scoped_ptr.hpp>

#include "LibUFEMDemo.hpp"
#include "UFEM/LSSActionUnsteady.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

namespace cf3 {
namespace UFEM {
namespace demo {

/// Solves the heat conduction equations using a hand-coded loop, only on triangular elements.
/// Intended as a demo only, use NavierStokesSteady for real simulations
class UFEM_API NavierStokesManual : public LSSActionUnsteady
{
public: // functions

  /// Contructor
  /// @param name of the component
  NavierStokesManual ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "NavierStokesManual"; }

  virtual void execute();
  
  /// Override so we can take action when the initial condition manager is set
  virtual void on_initial_conditions_set(InitialConditions& initial_conditions);
};

} // demo
} // UFEM
} // cf3


#endif // cf3_UFEM_NavierStokesManual_hpp
