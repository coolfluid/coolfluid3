// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_ForAllCells_hpp
#define cf3_solver_actions_ForAllCells_hpp


#include "solver/actions/Loop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

/////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API ForAllCells : public Loop
{
public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  ForAllCells ( const std::string& name );

  /// Virtual destructor
  virtual ~ForAllCells() {}

  /// Get the class name
  static std::string type_name () { return "ForAllCells"; }

  // functions specific to the ForAllCells component

  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_actions_ForAllCells_hpp
