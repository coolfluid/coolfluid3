// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CLoop_hpp
#define cf3_solver_actions_CLoop_hpp

#include "solver/actions/LibActions.hpp"
#include "solver/Action.hpp"
#include "solver/actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  namespace mesh
  {
    class Region;
  }

namespace solver {
namespace actions {


/////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API CLoop : public solver::Action
{
public: // typedefs

  /// provider
  typedef boost::shared_ptr< CLoop > Ptr;
  typedef boost::shared_ptr< CLoop const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CLoop ( const std::string& name );

  void trigger_Regions();

  /// Virtual destructor
  virtual ~CLoop() {}

  /// Get the class name
  static std::string type_name () { return "CLoop"; }

  // functions specific to the CLoop component

  CLoopOperation& create_loop_operation(const std::string action_provider);

  virtual const CLoopOperation& action(const std::string& name) const;

  virtual CLoopOperation& action(const std::string& name);

  virtual void execute() = 0;
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_CLoop_hpp
