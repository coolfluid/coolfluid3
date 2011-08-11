// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RiemannSolvers_RiemannSolver_hpp
#define CF_RiemannSolvers_RiemannSolver_hpp

////////////////////////////////////////////////////////////////////////////////


#include "Math/MatrixTypes.hpp"
#include "RiemannSolvers/src/RiemannSolvers/LibRiemannSolvers.hpp"

namespace CF {
namespace Solver {
  class State;
  class Physics;
}
namespace RiemannSolvers {

////////////////////////////////////////////////////////////////////////////////

/// @author Willem Deconinck
class RiemannSolvers_API RiemannSolver : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<RiemannSolver> Ptr;
  typedef boost::shared_ptr<RiemannSolver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  RiemannSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~RiemannSolver();

  /// Get the class name
  static std::string type_name () { return "RiemannSolver"; }

  // functions specific to the RiemannSolver component
  RealVector interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal);

  virtual void solve(const RealVector& left, const RealVector& right, const RealVector& normal, 
             RealVector& flux, Real& left_wave_speed, Real& right_wave_speed) = 0;
  
protected:

  boost::weak_ptr<Solver::State> m_sol_state;
  
};

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RiemannSolvers_RiemannSolver_hpp
