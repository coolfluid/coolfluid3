// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RiemannSolvers_AUSMplusUp_hpp
#define cf3_RiemannSolvers_AUSMplusUp_hpp

////////////////////////////////////////////////////////////////////////////////

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"
#include "Physics/NavierStokes/NavierStokes2D.hpp"
#include "RiemannSolvers/RiemannSolvers/RiemannSolver.hpp"

namespace cf3 {
namespace RiemannSolvers {

////////////////////////////////////////////////////////////////////////////////

class RiemannSolvers_API AUSMplusUp : public RiemannSolver
{
public:
  typedef boost::shared_ptr< AUSMplusUp >       Ptr;
  typedef boost::shared_ptr< AUSMplusUp const > ConstPtr;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// Contructor
  /// @param name of the component
  AUSMplusUp ( const std::string& name );

  /// Virtual destructor
  virtual ~AUSMplusUp();

  /// type name
  static std::string type_name() { return "AUSMplusUp"; }

  virtual void compute_interface_flux_and_wavespeeds(const RealVector& left, const RealVector& right, const RealVector& coords, const RealVector& normal,
                                                     RealVector& flux, RealVector& wave_speeds);

  virtual void compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& coords, const RealVector& normal,
                                      RealVector& flux);

public:
  Real M1(Real Mach, char chsign);
  Real M2(Real Mach, char chsign);
  Real M4(Real Mach, char chsign);

  Real P5(Real Mach, Real alpha, char chsign);

private:

  void trigger_physical_model();

private:

  std::auto_ptr<physics::NavierStokes::NavierStokes2D::Properties> p_left;
  std::auto_ptr<physics::NavierStokes::NavierStokes2D::Properties> p_right;
  std::auto_ptr<physics::NavierStokes::NavierStokes2D::Properties> p_avg;
  RealVector coord;
  RealMatrix grads;
  RealMatrix f_left;
  RealMatrix f_right;

  RealVector eigenvalues;
  RealMatrix left_eigenvectors;
  RealMatrix right_eigenvectors;
  RealMatrix abs_jacobian;


  Real  m_fa, m_CoeffKu, m_CoeffKp, m_Coeffsigma, m_Machinf, m_Beta;

  // Operator to calculate the absolute value
  struct Abs : public physics::UnaryRealOp
  {
    virtual Real operator() ( const Real& r ) const { return std::abs(r); };
  } abs;
};

////////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_RiemannSolvers_laxfriedrich_hpp
