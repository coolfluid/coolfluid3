// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_ForAllNodes2_hpp
#define cf3_solver_actions_ForAllNodes2_hpp


#include "solver/actions/Loop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

/////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API ForAllNodes2 : public Loop
{
public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  ForAllNodes2 ( const std::string& name );

  /// Virtual destructor
  virtual ~ForAllNodes2() {}

  /// Get the class name
  static std::string type_name () { return "ForAllNodes2"; }

  // functions specific to the ForAllNodes2 component

  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_ForAllNodes2_hpp
