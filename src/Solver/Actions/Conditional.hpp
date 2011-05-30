// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Conditional_hpp
#define CF_Solver_Conditional_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/Actions/LibActions.hpp"
#include "Common/CAction.hpp"

namespace CF {
namespace Solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// Action that gets executed Conditionalally
/// @author Tiago Quintino
class Solver_Actions_API Conditional : public Common::CAction {

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
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_Conditional_hpp
