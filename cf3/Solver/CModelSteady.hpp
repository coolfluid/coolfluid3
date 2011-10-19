// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_CModelSteady_hpp
#define cf3_Solver_CModelSteady_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CModel.hpp"
#include "Solver/LibSolver.hpp"

namespace cf3 {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// CModelSteady models a steady PDE problem
/// @author Tiago Quintino
class Solver_API CModelSteady : public Solver::CModel {

public: // typedefs

  typedef boost::shared_ptr<CModelSteady> Ptr;
  typedef boost::shared_ptr<CModelSteady const> ConstPtr;

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

} // Solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Solver_CModelSteady_hpp
