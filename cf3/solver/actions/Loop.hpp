// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Loop_hpp
#define cf3_solver_actions_Loop_hpp

#include "solver/actions/LibActions.hpp"
#include "solver/Action.hpp"
#include "solver/actions/LoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  namespace mesh
  {
    class Region;
  }

namespace solver {
namespace actions {


/////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API Loop : public solver::Action
{
public: // functions

  /// Contructor
  /// @param name of the component
  Loop ( const std::string& name );

  void trigger_Regions();

  /// Virtual destructor
  virtual ~Loop() {}

  /// Get the class name
  static std::string type_name () { return "Loop"; }

  // functions specific to the Loop component

  LoopOperation& create_loop_operation(const std::string action_provider);

  virtual const LoopOperation& action(const std::string& name) const;

  virtual LoopOperation& action(const std::string& name);

  virtual void execute() = 0;
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_Loop_hpp
