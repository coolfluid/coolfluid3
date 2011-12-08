// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CForAllCells_hpp
#define cf3_solver_actions_CForAllCells_hpp


#include "solver/actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

/////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API CForAllCells : public CLoop
{
public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllCells ( const std::string& name );

  /// Virtual destructor
  virtual ~CForAllCells() {}

  /// Get the class name
  static std::string type_name () { return "CForAllCells"; }

  // functions specific to the CForAllCells component

  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_actions_CForAllCells_hpp
