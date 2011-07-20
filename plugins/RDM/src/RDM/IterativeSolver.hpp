// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_IterativeSolver_hpp
#define CF_RDM_IterativeSolver_hpp

#include "Solver/ActionDirector.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {
namespace RDM {


/////////////////////////////////////////////////////////////////////////////////////

class RDM_API IterativeSolver : public CF::Solver::ActionDirector {

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

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

private: // functions

  /// @returns true if any of the stop criteria is achieved
  bool stop_condition();

private: // data

  /// set of actions called every iteration before non-linear solve
  Common::CActionDirector::Ptr m_pre_actions;
  /// set of actions called every iteration to update the solution
  Common::CActionDirector::Ptr m_update;
  /// set of actions called every iteration after non-linear solve
  Common::CActionDirector::Ptr m_post_actions;

};

/////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF

#endif // CF_RDM_IterativeSolver_hpp
