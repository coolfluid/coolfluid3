// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_Solver_hpp
#define cf3_sdm_Solver_hpp

#include "common/Action.hpp"

#include "sdm/LibSDM.hpp"

namespace cf3 {
namespace common { class Action; class ActionDirector; }
namespace mesh   { class Dictionary; class Field; }
namespace solver { class History; class Time; }
namespace sdm {

/////////////////////////////////////////////////////////////////////////////////////

class sdm_API Solver : public common::Action {

public: // functions

  /// Contructor
  /// @param name of the component
  Solver ( const std::string& name );

  /// Virtual destructor
  virtual ~Solver() {}

  /// Get the class name
  static std::string type_name () { return "Solver"; }

  common::ActionDirector& pre_update()    { return *m_pre_update; }
  common::ActionDirector& post_update()   { return *m_post_update; }

  const Handle< solver::History >& history() { return m_history; }

  virtual void execute();

  virtual void setup() {}

  virtual void step() = 0;

  virtual void advance() {}

  virtual std::string iteration_summary();

  bool stop_condition();
protected: // functions

  /// raises the event when iteration done
  void raise_iteration_done();

  void config_solution();

protected: // data

  Handle<Component> m_time_integration;
  /// set of actions called every iteration before non-linear solve
  Handle<common::ActionDirector> m_pre_update;
  /// set of actions called every iteration after non-linear solve
  Handle<common::ActionDirector> m_post_update;

  Handle< mesh::Field > m_solution;
  Handle< mesh::Dictionary > m_dict;
  Handle< solver::History > m_history;  ///< Component tracking history of several variables
  Handle< solver::Time > m_time;

};

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3

#endif // cf3_sdm_Solver_hpp
