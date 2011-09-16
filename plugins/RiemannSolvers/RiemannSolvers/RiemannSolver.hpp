// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RiemannSolvers_RiemannSolver_hpp
#define CF_RiemannSolvers_RiemannSolver_hpp

////////////////////////////////////////////////////////////////////////////////


#include "Math/MatrixTypes.hpp"
#include "RiemannSolvers/LibRiemannSolvers.hpp"

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

  /// Compute interface flux and wavespeeds
  virtual void compute_interface_flux_and_wavespeeds(const RealVector& left, const RealVector& right, const RealVector& normal,
                                                     RealVector& flux, RealVector& wave_speeds) = 0;

  /// Compute interface flux
  virtual void compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal,
                                      RealVector& flux) = 0;

protected:

  Physics::Variables& solution_vars() const { return *m_solution_vars.lock(); }
  Physics::PhysModel& model() const { return *m_model.lock(); }

private:
  boost::weak_ptr<Physics::PhysModel> m_model;
  boost::weak_ptr<Physics::Variables> m_solution_vars;
};

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RiemannSolvers_RiemannSolver_hpp
