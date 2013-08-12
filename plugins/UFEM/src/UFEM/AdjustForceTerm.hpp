// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_AdjustForceTerm_hpp
#define cf3_UFEM_AdjustForceTerm_hpp

#include "common/Option.hpp"

#include "solver/Time.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"

#include "LibUFEM.hpp"

namespace cf3 {

namespace UFEM {

/// AdjustForceTerm: Adjust force term proportionally to the difference with some target criterion (such as the bulk velocity)
class UFEM_API AdjustForceTerm : public solver::actions::Proto::ProtoAction
{

public: // functions

  /// Contructor
  /// @param name of the component
  AdjustForceTerm ( const std::string& name );

  virtual ~AdjustForceTerm();

  /// Get the class name
  static std::string type_name () { return "AdjustForceTerm"; }

  void execute();
  
private:
  
  void trigger_bulk_velocity_component();
  void trigger_bulk_velocity();
  
  Real m_target_velocity;
  Real m_current_velocity;
  Real m_dt;
  Real m_relaxation;
  Real m_max_force;
  Uint m_direction;
  
  Handle<common::Component> m_bulk_velocity_computer;
  Handle<solver::Time> m_time;
  common::Option::TriggerID m_bulk_velocity_trigger_id;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_AdjustForceTerm_hpp
