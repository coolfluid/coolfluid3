// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Solver_CSolveSystem_hpp
#define cf3_Solver_CSolveSystem_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CAction.hpp"

#include "Solver/Actions/LibActions.hpp"

namespace cf3 {
  namespace Math { namespace LSS { class System; } }
namespace Solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// CSolveSystem wraps a linear system solver in an action that will execute the solve
/// @author Bart Janssens
class Solver_Actions_API CSolveSystem : public common::CAction
{

public: // typedefs

  typedef boost::shared_ptr<CSolveSystem> Ptr;
  typedef boost::shared_ptr<CSolveSystem const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSolveSystem ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "CSolveSystem"; }

  /// Run the underlying linear system solver
  void execute();
  
private:
  boost::weak_ptr<Math::LSS::System> m_lss;
};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Solver_CSolveSystem_hpp
