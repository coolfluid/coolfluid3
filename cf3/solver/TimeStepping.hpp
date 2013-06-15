// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_TimeStepping_hpp
#define cf3_solver_TimeStepping_hpp

#include "solver/LibSolver.hpp"
#include "common/ActionDirector.hpp"

namespace cf3 {
namespace solver {

  class Time;
  class History;

/////////////////////////////////////////////////////////////////////////////////////

/// @brief Time Stepping component
/// @author Willem Deconinck
/// This component needs to have configured a end_time, and a time_step
/// It then executes exactly as many time-steps necessary to reach the
/// end_time with the given time_step. \n
/// pre_actions() and post_actions() hooks are available to perform actions
/// before and after the time-step execution. \n
/// A history file by default called "timestepping.tsv" is written every
/// step, containing timing and memory information per step.
/// This information is also given in the info stream.
class solver_API TimeStepping : public common::ActionDirector {

public: // functions

  /// Contructor
  /// @param name of the component
  TimeStepping ( const std::string& name );

  // Virtual destructor
  virtual ~TimeStepping() {}

  /// @brief Get the class name
  static std::string type_name () { return "TimeStepping"; }

  /// @brief do a step
  /// @post property "finished" is assigned to true if no more
  ///       steps are to be made
  virtual void do_step();

  // Execute this action
  virtual void execute ();

  /// @brief Actions to execute before the time step
  common::ActionDirector& pre_actions()  { return *m_pre_actions; }

  /// @brief Actions to execute after the time step
  common::ActionDirector& post_actions() { return *m_post_actions; }

  /// @brief Access to the history
  const Handle< solver::History >& history() { return m_history; }

  void add_time( const Handle<solver::Time>& time );

  bool finished();
  bool not_finished();

public: // signals

  /// @brief Do a step
  void signal_do_step( common::SignalArgs& args);

private: // functions

  /// @brief configure end_time in the m_time component
  void config_end_time();

  /// @brief configure time_step in the m_time component
  void config_time_step();

  /// @returns true if any of the stop criteria is achieved
  bool stop_condition();

  /// raises event when timestep is done
  void raise_timestep_done();

private: // data

  std::vector< Handle< solver::Time > > m_times;           ///< component tracking time
  Handle< common::ActionDirector > m_pre_actions;    ///< set of actions before non-linear solve
  Handle< common::ActionDirector > m_post_actions;   ///< set of actions after non-linear solve
  Handle< solver::History >        m_history;        ///< Component tracking history of several variables
};

/////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

#endif // cf3_solver_TimeStepping_hpp
