// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CForAllFaces_hpp
#define cf3_solver_actions_CForAllFaces_hpp


#include "solver/actions/CLoop.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

/////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API CForAllFaces : public CLoop
{
public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CForAllFaces ( const std::string& name );

  /// Virtual destructor
  virtual ~CForAllFaces() {}

  /// Get the class name
  static std::string type_name () { return "CForAllFaces"; }

  // functions specific to the CForAllFaces component

  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_actions_CForAllFaces_hpp
