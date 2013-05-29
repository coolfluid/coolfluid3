// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_PDESolver_hpp
#define cf3_solver_PDESolver_hpp

#include "common/Action.hpp"
#include "solver/LibSolver.hpp"

/////////////////////////////////////////////////////////////////////////////////////

// Forward declarations
namespace cf3 {
  namespace common { 
    class Action; 
    class ActionDirector; 
  }
  namespace mesh { 
    class Dictionary; 
    class Field; 
  }
  namespace solver { 
    class History; 
    class Time;
    class TimeStepComputer ;
    class ComputeLNorm;
    class PDE; 
  }
}

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver { 

/////////////////////////////////////////////////////////////////////////////////////

/// @brief Solver base class to solve Partial Differential Equations, defined in class PDE
/// @author Willem Deconinck
class solver_API PDESolver : public common::Action {

public: // functions

  /// Contructor
  /// @param name of the component
  PDESolver ( const std::string& name );

  /// Virtual destructor
  virtual ~PDESolver() {}

  /// Get the class name
  static std::string type_name () { return "PDESolver"; }

  const Handle< solver::History >& history() { return m_history; }

  const Handle< solver::ComputeLNorm >& norm_computer() { return m_norm_computer; }

  virtual void execute();

  virtual void setup() {}

  void do_iteration();

  void solve_time_step(const Real time_step);

  void solve_iterations(const Uint nb_iterations);

  virtual void step() = 0;

  virtual void iteration_summary();

  bool stop_condition();

  const Handle<solver::TimeStepComputer> time_step_computer() { return m_time_step_computer; }

public: // signals

  void signal_solve_time_step( common::SignalArgs& args );
  void signature_solve_time_step( common::SignalArgs& args );

  void signal_solve_iterations( common::SignalArgs& args );
  void signature_solve_iterations( common::SignalArgs& args );

protected: // functions

  /// raises the event when iteration done
  void raise_iteration_done();

  void config_solution();

private: // functions

  void set_time_step_computer();

  void config_time_step_computer();

  void config_history();

  void config_norm_computer();

protected: // data

  Handle<solver::PDE> m_pde;

  Handle<solver::TimeStepComputer> m_time_step_computer;
  
  /// set of actions called every iteration before non-linear solve
  Handle<common::ActionDirector> m_pre_iteration;
  /// set of actions called every iteration after non-linear solve
  Handle<common::ActionDirector> m_post_iteration;
  
  Handle<common::ActionDirector> m_prepare_function;

  Handle<common::ActionDirector> m_function;

  Handle<common::ActionDirector> m_pre_update;
  Handle<common::ActionDirector> m_post_update;

  Handle< solver::History > m_history;  ///< Component tracking history of several variables

  Handle< solver::ComputeLNorm > m_norm_computer;

};

/////////////////////////////////////////////////////////////////////////////////////


} // solver
} // cf3

#endif // cf3_solver_PDESolver_hpp
