// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CSolver_hpp
#define CF_Solver_CSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Solver component class
/// Abstract base class for solvers, requiring the implementation of the solve() function
/// @author Tiago Quintino
/// @author Willem Deconinck
class Solver_API CSolver : public Common::Component {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CSolver> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CSolver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~CSolver();

  /// Get the class name
  static std::string type_name () { return "CSolver"; }

  // functions specific to the CSolver component

  /// Calls the concrete implementation of the solver
  virtual void solve() = 0;

  /// @name SIGNALS
  //@{

  /// Signal to start solving
  void signal_solve ( Common::SignalArgs& node );

  //@} END SIGNALS

};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CSolver_hpp
