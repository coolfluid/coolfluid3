// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RiemannSolvers_LaxFriedrich_hpp
#define cf3_RiemannSolvers_LaxFriedrich_hpp

////////////////////////////////////////////////////////////////////////////////

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"
#include "RiemannSolvers/RiemannSolvers/RiemannSolver.hpp"

namespace cf3 {
namespace RiemannSolvers {

////////////////////////////////////////////////////////////////////////////////

class RiemannSolvers_API LaxFriedrich : public RiemannSolver
{
public:
  typedef boost::shared_ptr< LaxFriedrich >       Ptr;
  typedef boost::shared_ptr< LaxFriedrich const > ConstPtr;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// Contructor
  /// @param name of the component
  LaxFriedrich ( const std::string& name );

  /// Virtual destructor
  virtual ~LaxFriedrich();

  /// type name
  static std::string type_name() { return "LaxFriedrich"; }


  virtual void compute_interface_flux_and_wavespeeds(const RealVector& left, const RealVector& right, const RealVector& coords, const RealVector& normal,
                                                     RealVector& flux, RealVector& wave_speeds);

  virtual void compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& coords, const RealVector& normal,
                                      RealVector& flux);

private:

  void trigger_physical_model();

private:

  std::auto_ptr<physics::Properties> p_left;
  std::auto_ptr<physics::Properties> p_right;
  std::auto_ptr<physics::Properties> p_avg;
  RealVector coord;
  RealMatrix grads;
  RealMatrix f_left;
  RealMatrix f_right;
  RealVector eigenvalues;
  RealVector eigenvalues_left;
  RealVector eigenvalues_right;
  RealMatrix mat_eigenvalues_left;
  RealMatrix mat_eigenvalues_right;
  RealMatrix left_eigenvectors;
  RealMatrix right_eigenvectors;
  RealMatrix abs_jacobian;

  RealVector laxfriedrich_flux;
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

#endif // cf3_RiemannSolvers_laxfriedrich_hpp
