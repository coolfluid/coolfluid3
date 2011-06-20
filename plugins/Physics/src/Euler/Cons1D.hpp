// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Euler_Cons1D_hpp
#define CF_Euler_Cons1D_hpp

#include "Common/Log.hpp"
#include "Solver/State.hpp"
#include "Euler/Physics.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Euler {

////////////////////////////////////////////////////////////////////////////////

/// @brief Base class to interface the Cons1D
/// @author Willem Deconinck
class Euler_API Cons1D : public Solver::State {

public: // typedefs

  typedef boost::shared_ptr<Cons1D> Ptr;
  typedef boost::shared_ptr<Cons1D const> ConstPtr;

public: // functions

  /// Contructor
  Cons1D( const std::string& name = type_name() ) : Solver::State(name)
  {
    m_var_names.resize(3);
    m_var_names[0]="rho";
    m_var_names[1]="rhoVx";
    m_var_names[2]="rhoE";
  }

  /// Virtual destructor
  virtual ~Cons1D() {}

  /// Gets the Class name
  static std::string type_name() { return "Cons1D"; }

  virtual Solver::Physics create_physics()
  {
    Euler::Physics p;
    return static_cast<Solver::Physics>(p);
  }

  virtual Uint size() { return 3; }

  virtual void set_state( const RealVector& state, Solver::Physics& p)
  {
    typedef Euler::Physics P;
    CF_DEBUG_POINT;

    p.init();
    CF_DEBUG_POINT;

    p.set_var(P::gamma, 1.4);
    CF_DEBUG_POINT;

    p.set_var(P::R, 286.9);
    CF_DEBUG_POINT;

    p.set_var(P::rho, state[0]);
    CF_DEBUG_POINT;
    p.set_var(P::Vx,  state[1]/state[0]);
    CF_DEBUG_POINT;
    p.set_var(P::Vy,  0.);
    p.set_var(P::Vz,  0.);
    p.set_var(P::E ,  state[2]/state[0]);
    CF_DEBUG_POINT;

    CFinfo << p << CFendl;
    CFinfo << p.compute_var(P::gamma_minus_1) << CFendl;
    CF_DEBUG_POINT;
    p.set_var(P::p ,  (p.var(P::gamma)-1.)*state[2]-0.5*state[0]*p.compute_var(P::V2));

    CF_DEBUG_POINT;
  }

  virtual void get_state( Solver::Physics& p, RealVector& state)
  {
    typedef Euler::Physics P;

    state[0] = p.var(P::rho);
    state[1] = p.var(P::rho)*p.var(P::Vx);
    state[2] = p.var(P::rho)*p.compute_var(P::E);
  }


  virtual void compute_flux( Solver::Physics& p,
                            const RealVector& normal,
                            RealVector& flux)
  {
    typedef Euler::Physics P;
    const Real un = p.var(P::Vx) * normal[XX];
    flux[0] = p.var(P::rho) * un;
    flux[1] = p.var(P::rho) * p.var(P::Vx) * un + p.var(P::p);
    flux[2] = (p.var(P::rho) * p.compute_var(P::E) + p.var(P::p)) * un;
  }

  virtual void compute_fluxjacobian_right_eigenvectors( Solver::Physics& p,
                                                        const RealVector& normal,
                                                        RealMatrix& rv)
  {
    typedef Euler::Physics P;

    const Real r=p.var(P::rho);
    const Real u=p.var(P::Vx);
    const Real h=p.compute_var(P::H);
    const Real a=p.compute_var(P::a);
    const Real nx=normal[XX];

    rv <<
          1.,           0.5*r/a,             0.5*r/a,
          u,            0.5*r/a*(u+a*nx),    0.5*r/a*(u-a*nx),
          0.5*u*u,      0.5*r/a*(h+u*a*nx),  0.5*r/a*(h-u*a*nx);
  }

  virtual void compute_fluxjacobian_left_eigenvectors( Solver::Physics& p,
                                                       const RealVector& normal,
                                                       RealMatrix& lv)
  {
    typedef Euler::Physics P;

    const Real gm1=p.var(P::gamma_minus_1);
    const Real r=p.var(P::rho);
    const Real u=p.var(P::Vx);
    const Real h=p.compute_var(P::H);
    const Real a=p.compute_var(P::a);
    const Real nx=normal[XX];

    lv <<
            1.-0.5*gm1*u*u/(a*a),              gm1*u/(a*a),         -gm1/(a*a),
            a/r*(0.5*gm1*u*u/(a*a)-u*nx/a),    1./r*(nx-gm1*u/a),    gm1/(r*a),
            a/r*(0.5*gm1*u*u/(a*a)+u*nx/a),    -1./r*(nx+gm1*u/a),   gm1/(r*a);
    }

  virtual void compute_fluxjacobian_eigenvalues( Solver::Physics& p,
                                                 const RealVector& normal,
                                                 RealVector& ev)
  {
    typedef Euler::Physics P;

    const Real un = p.var(P::Vx) * normal[XX];

    ev[0] = un;
    ev[1] = un+p.compute_var(P::a);
    ev[2] = un-p.var(P::a);
  }

  virtual Real max_eigen_value ( Solver::Physics& p, const RealVector& normal )
  {
    typedef Euler::Physics P;

    return p.var(P::Vx) * normal[XX] + p.compute_var(P::a);
  }

  virtual void linearize( std::vector<Solver::Physics>& states, Solver::Physics& p )
  {
    throw Common::NotImplemented(FromHere(),"");
  }

}; // Cons1D

////////////////////////////////////////////////////////////////////////////////

} // Euler
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Euler_Cons1D_hpp
