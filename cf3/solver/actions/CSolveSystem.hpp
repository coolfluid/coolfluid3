// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_CSolveSystem_hpp
#define cf3_solver_CSolveSystem_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"

#include "solver/actions/LibActions.hpp"

namespace cf3 {
  namespace math { namespace LSS { class System; } }
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// CSolveSystem wraps a linear system solver in an action that will execute the solve
/// @author Bart Janssens
class solver_actions_API CSolveSystem : public common::Action
{

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CSolveSystem ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "CSolveSystem"; }

  /// Run the underlying linear system solver
  void execute();
  
private:
  Handle<math::LSS::System> m_lss;
};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_CSolveSystem_hpp
