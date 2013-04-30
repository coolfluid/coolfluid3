// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_ForAllFaces_hpp
#define cf3_solver_actions_ForAllFaces_hpp


#include "solver/actions/Loop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

/////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API ForAllFaces : public Loop
{
public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  ForAllFaces ( const std::string& name );

  /// Virtual destructor
  virtual ~ForAllFaces() {}

  /// Get the class name
  static std::string type_name () { return "ForAllFaces"; }

  // functions specific to the ForAllFaces component

  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_actions_ForAllFaces_hpp
