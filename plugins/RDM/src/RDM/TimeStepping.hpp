// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_TimeStepping_hpp
#define cf3_RDM_TimeStepping_hpp

#include "solver/ActionDirector.hpp"

#include "RDM/LibRDM.hpp"

namespace cf3 {

  namespace solver { class CTime; }

namespace RDM {


/////////////////////////////////////////////////////////////////////////////////////

class RDM_API TimeStepping : public cf3::solver::ActionDirector {

public: // typedefs




public: // functions
  /// Contructor
  /// @param name of the component
  TimeStepping ( const std::string& name );

  /// Virtual destructor
  virtual ~TimeStepping() {}

  /// Get the class name
  static std::string type_name () { return "TimeStepping"; }

  /// execute the action
  virtual void execute ();

  common::ActionDirector& pre_actions()  { return *m_pre_actions; }
  common::ActionDirector& post_actions() { return *m_post_actions; }

  cf3::solver::CTime&       time()         { return *m_time; }

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

private: // functions

  /// @returns true if any of the stop criteria is achieved
  bool stop_condition();
  /// raises event when timestep is done
  void raise_timestep_done();

private: // data

  Handle< solver::CTime > m_time;   ///< component tracking time

  Handle< common::ActionDirector > m_pre_actions;  ///< set of actions before non-linear solve

  Handle< common::ActionDirector > m_post_actions; ///< set of actions after non-linear solve

};

/////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3

#endif // cf3_RDM_TimeStepping_hpp
