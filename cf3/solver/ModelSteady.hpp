// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_ModelSteady_hpp
#define cf3_solver_ModelSteady_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Model.hpp"
#include "solver/LibSolver.hpp"

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// ModelSteady models a steady PDE problem
/// @author Tiago Quintino
class solver_API ModelSteady : public solver::Model {

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  ModelSteady ( const std::string& name );

  /// Virtual destructor
  virtual ~ModelSteady();

  /// Get the class name
  static std::string type_name () { return "ModelSteady"; }
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_ModelSteady_hpp
