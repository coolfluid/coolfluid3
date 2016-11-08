// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_UnsteadyAction_hpp
#define cf3_UFEM_UnsteadyAction_hpp

#include "solver/Action.hpp"
#include "solver/Time.hpp"

#include "LibUFEM.hpp"
#include "LSSAction.hpp"

namespace cf3 {
namespace solver { class Time; }
namespace UFEM {

/// UnsteadyAction, allowing direct access to the time step
class UFEM_API UnsteadyAction : public solver::Action
{
public: // functions

  /// Contructor
  /// @param name of the component
  UnsteadyAction ( const std::string& name );

  virtual ~UnsteadyAction();

  /// Get the class name
  static std::string type_name () { return "UnsteadyAction"; }

  /// Reference to the timestep
  Real& dt();

  /// Reference to the inverse timestep, linked to the model time step
  Real& invdt();

  const solver::Time& time() const;

protected:
  Real m_dt, m_invdt;

private:

  void trigger_time();
  void trigger_timestep();

  Handle<solver::Time> m_time;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_UnsteadyAction_hpp
