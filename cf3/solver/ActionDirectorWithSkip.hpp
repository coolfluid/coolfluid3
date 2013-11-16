// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_ActionDirectorWithSkip_hpp
#define cf3_solver_ActionDirectorWithSkip_hpp

#include "solver/ActionDirector.hpp"

#include "mesh/Region.hpp"

#include "solver/LibSolver.hpp"

namespace cf3 {

namespace mesh { class Mesh; }
namespace physics { class PhysModel; }

namespace solver {

class Solver;
class Time;

/////////////////////////////////////////////////////////////////////////////////////

class solver_API ActionDirectorWithSkip : public solver::ActionDirector {

public: // functions

  /// Contructor
  /// @param name of the component
  ActionDirectorWithSkip ( const std::string& name );

  /// Virtual destructor
  virtual ~ActionDirectorWithSkip();

  /// Get the class name
  static std::string type_name () { return "ActionDirectorWithSkip"; }
  
  virtual void execute();

private:
  // Interval over which to skip execution
  Uint m_interval;
  // Total number of executions
  Uint m_count;

};

/////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

#endif // cf3_solver_ActionDirectorWithSkip_hpp
