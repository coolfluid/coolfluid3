// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_Wizard_hpp
#define cf3_solver_Wizard_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/LibSolver.hpp"

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// Wizard component class
/// Derived classes typically help users configure
/// and setup a simulation
/// @author Willem Deconinck
class solver_API Wizard : public common::Component {

public: // typedefs

  /// type of pointer to Component
  
  /// type of pointer to constant Component
  

public: // functions

  /// Contructor
  /// @param name of the component
  Wizard ( const std::string& name );

  /// Virtual destructor
  virtual ~Wizard();

  /// Get the class name
  static std::string type_name () { return "Wizard"; }

  virtual std::string wizard_name() { return "wizard"; }
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_Wizard_hpp
