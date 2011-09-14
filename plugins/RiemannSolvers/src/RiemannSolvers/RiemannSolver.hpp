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
namespace Physics { class Variables; class PhysModel;}
namespace RiemannSolvers {

////////////////////////////////////////////////////////////////////////////////

/// @author Willem Deconinck
class RiemannSolvers_API RiemannSolver : public Common::Component
{
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
  virtual void compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal,
                                      RealVector& flux, Real& wave_speed) = 0;

  Physics::Variables& solution_vars() const { return *m_solution_vars.lock(); }

  Physics::PhysModel& model() const { return *m_model.lock(); }
protected:

  boost::weak_ptr<Physics::PhysModel> m_model;
  boost::weak_ptr<Physics::Variables> m_solution_vars;
};

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RiemannSolvers_RiemannSolver_hpp
