// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_VelocityGradient_hpp
#define cf3_UFEM_VelocityGradient_hpp

#include "InitialConditions.hpp"

#include "LibUFEM.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3 {
namespace UFEM {

/// Compute a field containing the velocity gradient tensor and the divergence
class VelocityGradient : public solver::actions::Proto::ProtoAction
{
public: // functions
  /// Contructor
  /// @param name of the component
  VelocityGradient( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "VelocityGradient"; }
  
  virtual void execute();

private:
  void trigger_initial_conditions();
  virtual void on_regions_set();
  Handle<InitialConditions> m_initial_conditions;
  Handle<common::Component> m_node_valence;
  Handle<solver::actions::Proto::ProtoAction> m_zero_fields;
  void setup_node_valence_regions();
};

} // UFEM
} // cf3


#endif // cf3_UFEM_VelocityGradient_hpp
