// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_TaylorGreen_hpp
#define cf3_UFEM_TaylorGreen_hpp

#include "math/MatrixTypes.hpp"

#include "solver/Time.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"

#include "LibUFEMParticles.hpp"

namespace cf3 {
  namespace solver { class Time; }
namespace UFEM {
namespace particles {

namespace detail {

// Taylor-Green model evaluation
struct TaylorGreenModel
{
  typedef Eigen::Matrix<Real, 2, 1> CoordsT;
  
  TaylorGreenModel() :
    Ua(0.3),
    Va(0.2),
    Vs(1.),
    D(0.5),
    nu(1.),
    beta(0.),
    tau(0.01)
  {
  }
  
  // Fluid X velocity
  Real ux(const Real t) const;
  // Fluid Y velocity
  Real uy(const Real t) const;
  // Particle X velocity
  Real vx(const Real t) const;
  // Particle Y velocity
  Real vy(const Real t) const;
  
  /// Coordinates at which to evaluate
  Real x,y;
  
  /// Coefficients for the fluid
  Real Ua, Va, Vs, D, nu;

  /// Coefficients for the particle dispersion
  Real beta, tau;
};

}
  
/// Store the SUPG coefficient values in a field
class UFEM_API TaylorGreen : public solver::actions::Proto::ProtoAction
{
public:
  /// Contructor
  /// @param name of the component
  TaylorGreen ( const std::string& name );

  virtual ~TaylorGreen();

  /// Get the class name
  static std::string type_name () { return "TaylorGreen"; }
  
private:
  Real tau_ps, tau_su, tau_bu;
  
  void trigger_time();
  void trigger_current_time();

  Handle<solver::Time> m_time;
  
  detail::TaylorGreenModel m_tg_model;
  RealVector m_tg_values;

  /// Previous time level
  Real m_t;
  /// Time step
  Real m_dt;
};

} // Particles
} // UFEM
} // cf3


#endif // cf3_UFEM_TaylorGreen_hpp
