// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_HeatConductionSteady_hpp
#define cf3_UFEM_HeatConductionSteady_hpp

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "LSSAction.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

namespace cf3 {

namespace UFEM {

/// Steady, linear heat conduction
class UFEM_API HeatConductionSteady : public LSSAction
{
public:
  // Component basics:
  HeatConductionSteady ( const std::string& name );
  static std::string type_name () {return "HeatConductionSteady";}

private:
  // Set default initial conditions
  virtual void on_initial_conditions_set(InitialConditions& initial_conditions);
  // Triggered on option changes
  void trigger();
  // System assembly
  Handle<solver::actions::Proto::ProtoAction> m_assembly;
  // Solution update
  Handle<solver::actions::Proto::ProtoAction> m_update;

  PhysicsConstant lambda_s;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_HeatConductionSteady_hpp
