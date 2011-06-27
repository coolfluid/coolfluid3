// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_AdvectionDiffusion_State1D_hpp
#define CF_AdvectionDiffusion_State1D_hpp

#include "Solver/State.hpp"
#include "AdvectionDiffusion/Physics.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace AdvectionDiffusion {

////////////////////////////////////////////////////////////////////////////////

/// @brief Base class to interface the State1D
/// @author Willem Deconinck
class AdvectionDiffusion_API State1D : public Solver::State {

public: // typedefs

  typedef boost::shared_ptr<State1D> Ptr;
  typedef boost::shared_ptr<State1D const> ConstPtr;

public: // functions

  /// Contructor
  State1D( const std::string& name = type_name() ) : Solver::State(name)
  {
    m_var_names.resize(1,"Q");
    m_advection_speed = 1.;
  }

  /// Virtual destructor
  virtual ~State1D() {}

  /// Gets the Class name
  static std::string type_name() { return "State1D"; }

  virtual boost::shared_ptr<Solver::Physics> create_physics()
  {
    boost::shared_ptr<Solver::Physics> p (new AdvectionDiffusion::Physics);
    return p;
  }

  virtual Uint size() { return 1; }

  virtual void set_state( const RealVector& state, Solver::Physics& p)
  {
    p.init();
    p.set_var(AdvectionDiffusion::Physics::Vx, m_advection_speed);
    p.set_var(AdvectionDiffusion::Physics::S , state[0]);
  }

  virtual void get_state( Solver::Physics& p, RealVector& state)
  {
    state[0] = p.var(AdvectionDiffusion::Physics::S);
  }


  virtual void compute_flux( Solver::Physics& p,
                            const RealVector& normal,
                            RealVector& flux)
  {
    flux[0] = p.var(Physics::Vx)*p.var(Physics::S)*normal[XX];
  }

  virtual void compute_fluxjacobian_right_eigenvectors( Solver::Physics& p,
                                                        const RealVector& normal,
                                                        RealMatrix& rv)
  {
    rv(0,0) = 1.;
  }

  virtual void compute_fluxjacobian_left_eigenvectors( Solver::Physics& p,
                                                       const RealVector& normal,
                                                       RealMatrix& lv)
  {
    lv(0,0) = 1.;
  }

  virtual void compute_fluxjacobian_eigenvalues( Solver::Physics& p,
                                                 const RealVector& normal,
                                                 RealVector& ev)
  {
    ev[0] = p.var(Physics::Vx) * normal[XX];
  }

  virtual Real max_eigen_value ( Solver::Physics& p, const RealVector& normal )
  {
    return std::abs(p.var(Physics::Vx) * normal[XX]);
  }

  virtual void linearize( std::vector<boost::shared_ptr<Solver::Physics> >& states, Solver::Physics& p )
  {
    Real S=0;
    for (Uint i=0; i<states.size(); ++i)
      S += states[i]->var(AdvectionDiffusion::Physics::S);
    S /= static_cast<Real>(states.size());

    p.init();
    p.set_var(AdvectionDiffusion::Physics::Vx, m_advection_speed);
    p.set_var(AdvectionDiffusion::Physics::S,  S );
  }

private:
  Real m_advection_speed;
}; // State1D

////////////////////////////////////////////////////////////////////////////////

} // AdvectionDiffusion
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_AdvectionDiffusion_State1D_hpp
