// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_IterativeSolver_hpp
#define cf3_RDM_IterativeSolver_hpp

#include "Solver/ActionDirector.hpp"

#include "RDM/LibRDM.hpp"

namespace cf3 {
namespace RDM {


/////////////////////////////////////////////////////////////////////////////////////

class RDM_API IterativeSolver : public cf3::Solver::ActionDirector {

public: // typedefs

  typedef boost::shared_ptr<IterativeSolver> Ptr;
  typedef boost::shared_ptr<IterativeSolver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  IterativeSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~IterativeSolver() {}

  /// Get the class name
  static std::string type_name () { return "IterativeSolver"; }

  /// execute the action
  virtual void execute ();

  common::ActionDirector& pre_actions()  { return *m_pre_actions; }
  common::ActionDirector& update()       { return *m_update; }
  common::ActionDirector& post_actions() { return *m_post_actions; }

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

private: // functions

  /// @returns true if any of the stop criteria is achieved
  bool stop_condition();
  /// raises the event when iteration done
  void raise_iteration_done();

private: // data

  /// set of actions called every iteration before non-linear solve
  common::ActionDirector::Ptr m_pre_actions;
  /// set of actions called every iteration to update the solution
  common::ActionDirector::Ptr m_update;
  /// set of actions called every iteration after non-linear solve
  common::ActionDirector::Ptr m_post_actions;

};

/////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3

#endif // cf3_RDM_IterativeSolver_hpp
