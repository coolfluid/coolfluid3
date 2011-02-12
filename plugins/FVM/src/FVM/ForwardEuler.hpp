// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_ForwardEuler_hpp
#define CF_FVM_ForwardEuler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLink.hpp"

#include "Solver/CIterativeSolver.hpp"

#include "FVM/ComputeUpdateCoefficient.hpp"
#include "FVM/LibFVM.hpp"

namespace CF {

namespace Solver { 
  class CDiscretization; 
  namespace Actions { 
    class CLoop; 
  }
}

namespace Mesh {
  class CField2;
}

namespace FVM {
  class ComputeUpdateCoefficient;

////////////////////////////////////////////////////////////////////////////////

/// RKRD iterative solver
/// @author Tiago Quintino
/// @author Willem Deconinck
class FVM_API ForwardEuler : public Solver::CIterativeSolver {

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
  
  Solver::CDiscretization& discretization_method();
  
private: // functions

  void trigger_Domain();
  void trigger_solution();

private: // data
  
  Common::CLink::Ptr m_solution;
  
  Common::CLink::Ptr m_residual;
  
  Common::CLink::Ptr m_advection;

  Common::CLink::Ptr m_update_coeff;

  boost::shared_ptr<ComputeUpdateCoefficient> m_compute_update_coefficient;
  
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_ForwardEuler_hpp
