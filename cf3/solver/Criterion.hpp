// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_Criterion_hpp
#define cf3_solver_Criterion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/LibSolver.hpp"

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

/// Criterion models a boolean criterion
/// @author Tiago Quintino
class solver_API Criterion : public common::Component
{
public: // functions

  /// Contructor
  /// @param name of the component
  Criterion ( const std::string& name );

  /// Virtual destructor
  virtual ~Criterion();

  /// Get the class name
  static std::string type_name () { return "Criterion"; }

  /// Return the state of the criterion
  virtual bool operator()() = 0;
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_Criterion_hpp
