// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RiemannSolvers_Roe_hpp
#define cf3_RiemannSolvers_Roe_hpp

////////////////////////////////////////////////////////////////////////////////

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"
#include "RiemannSolvers/RiemannSolver.hpp"

namespace cf3 {
namespace RiemannSolvers {

////////////////////////////////////////////////////////////////////////////////

class RiemannSolvers_API Roe : public RiemannSolver
{
public:
  
  

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// Contructor
  /// @param name of the component
  Roe ( const std::string& name );

  /// Virtual destructor
  virtual ~Roe();

  /// type name
  static std::string type_name() { return "Roe"; }


  virtual void compute_interface_flux_and_wavespeeds(const RealVector& left, const RealVector& right, const RealVector& coords, const RealVector& normal,
                                                     RealVector& flux, RealVector& wave_speeds);

  virtual void compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& coords, const RealVector& normal,
                                      RealVector& flux);

private:

  void trigger_physical_model();
  physics::Variables& roe_vars() { return *m_roe_vars; }

private:

  Handle<physics::Variables> m_roe_vars;
  std::auto_ptr<physics::Properties> p_left;
  std::auto_ptr<physics::Properties> p_right;
  std::auto_ptr<physics::Properties> p_avg;
  RealVector coord;
  RealMatrix grads;
  RealVector f_left;
  RealVector f_right;
  RealVector roe_left;
  RealVector roe_right;
  RealVector roe_avg;
  RealVector eigenvalues;
  RealMatrix left_eigenvectors;
  RealMatrix right_eigenvectors;
  RealMatrix abs_jacobian;

  RealVector central_flux;
  RealVector upwind_flux;

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

#endif // cf3_RiemannSolvers_Roe_hpp
