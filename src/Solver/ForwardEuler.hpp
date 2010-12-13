// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_ForwardEuler_hpp
#define CF_Solver_ForwardEuler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLink.hpp"

#include "Solver/CIterativeSolver.hpp"

namespace CF {
namespace Actions {
  class CLoop;
}
namespace Solver {

  class CDiscretization;

////////////////////////////////////////////////////////////////////////////////

/// Solver component class
/// @author Tiago Quintino
/// @author Willem Deconinck
class Solver_API ForwardEuler : public Solver::CIterativeSolver {

public: // typedefs

  typedef boost::shared_ptr<ForwardEuler> Ptr;
  typedef boost::shared_ptr<ForwardEuler const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  ForwardEuler ( const std::string& name );

  /// Virtual destructor
  virtual ~ForwardEuler();

  /// Get the class name
  static std::string type_name () { return "ForwardEuler"; }

  // functions specific to the ForwardEuler component
  
  virtual void solve();
  
  CDiscretization& discretization_method();
  
private: // functions

  void trigger_Domain();
  
private: // data
  
  Common::CLink::Ptr m_solution_field;
  
  Common::CLink::Ptr m_residual_field;
  
  Common::CLink::Ptr m_update_coeff_field;
  
  boost::shared_ptr<Actions::CLoop> m_take_step;
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_ForwardEuler_hpp
