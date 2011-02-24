// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CCriterion_hpp
#define CF_Solver_CCriterion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/Actions/LibActions.hpp"
#include "Common/CAction.hpp"

namespace CF {
namespace Solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// CCriterion models a Unsteady PDE problem
/// @author Tiago Quintino
class Solver_Actions_API CCriterion : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CCriterion> Ptr;
  typedef boost::shared_ptr<CCriterion const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CCriterion ( const std::string& name );

  /// Virtual destructor
  virtual ~CCriterion();

  /// Get the class name
  static std::string type_name () { return "CCriterion"; }

  /// Simulates this model
  virtual bool operator()() = 0;
};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CCriterion_hpp
