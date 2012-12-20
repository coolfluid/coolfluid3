// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_HeatConductionManual_hpp
#define cf3_UFEM_HeatConductionManual_hpp

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "LSSAction.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

namespace cf3 {

namespace UFEM {

/// Solves the heat conduction equations using a hand-coded loop, only on triangular elements.
/// Intended as a demo only, use HeatConductionSteady for real simulations
class UFEM_API HeatConductionManual : public LSSAction
{
public: // functions

  /// Contructor
  /// @param name of the component
  HeatConductionManual ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "HeatConductionManual"; }

private:
  virtual void on_initial_conditions_set(InitialConditions& initial_conditions);
};

} // UFEM
} // cf3


#endif // cf3_UFEM_HeatConductionManual_hpp
