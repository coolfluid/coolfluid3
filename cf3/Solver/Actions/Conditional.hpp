// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_Conditional_hpp
#define cf3_Solver_Conditional_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/Actions/LibActions.hpp"
#include "common/CAction.hpp"

namespace cf3 {
namespace Solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// Action that gets executed Conditionalally
/// @author Tiago Quintino
class Solver_Actions_API Conditional : public common::CAction {

public: // typedefs

  typedef boost::shared_ptr<Conditional> Ptr;
  typedef boost::shared_ptr<Conditional const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Conditional ( const std::string& name );

  /// Virtual destructor
  virtual ~Conditional();

  /// Get the class name
  static std::string type_name () { return "Conditional"; }

  /// executes child actions if the if criterion is met.
  /// If no criterion is added, default = true
  virtual void execute();

protected: // data

};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Solver_Conditional_hpp
