// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_CModelSteady_hpp
#define cf3_solver_CModelSteady_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/CModel.hpp"
#include "solver/LibSolver.hpp"

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// CModelSteady models a steady PDE problem
/// @author Tiago Quintino
class solver_API CModelSteady : public solver::CModel {

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CModelSteady ( const std::string& name );

  /// Virtual destructor
  virtual ~CModelSteady();

  /// Get the class name
  static std::string type_name () { return "CModelSteady"; }
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_CModelSteady_hpp
