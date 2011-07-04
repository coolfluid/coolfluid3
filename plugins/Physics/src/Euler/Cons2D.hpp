// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Euler_Cons2D_hpp
#define CF_Euler_Cons2D_hpp

#include "Solver/State.hpp"
#include "Physics/src/Euler/Physics.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Euler {

////////////////////////////////////////////////////////////////////////////////

/// @brief Base class to interface the Cons2D
/// @author Willem Deconinck
class Euler_API Cons2D : public Solver::State {

public: // typedefs

  typedef boost::shared_ptr<Cons2D> Ptr;
  typedef boost::shared_ptr<Cons2D const> ConstPtr;

public: // functions

  /// Contructor
  Cons2D( const std::string& name = type_name() ) : Solver::State(name)
  {
    m_var_names.resize(4);
    m_var_names[0]="rho";
    m_var_names[1]="rhoVx";
    m_var_names[2]="rhoVy";
    m_var_names[3]="rhoE";
  }

  /// Virtual destructor
  virtual ~Cons2D() {}

  /// Gets the Class name
  static std::string type_name() { return "Cons2D"; }

  virtual boost::shared_ptr<Solver::Physics> create_physics()
  {
    boost::shared_ptr<Solver::Physics> p (new Euler::Physics);
    return p;
  }

  virtual Uint size() { return 4; }

  virtual void set_state( const RealVector& state, Solver::Physics& p)
  {
    typedef Euler::Physics P;

    p.init();

    p.set_var(P::gamma, 1.4);
    p.set_var(P::R, 286.9);
    p.set_var(P::rho, state[0]);
    p.set_var(P::Vx,  state[1]/state[0]);
    p.set_var(P::Vy,  state[2]/state[0]);
    p.set_var(P::Vz,  0.);
    p.set_var(P::E ,  state[3]/state[0]);
    p.set_var(P::p , p.compute_var(P::gamma_minus_1)*(state[3]-0.5*state[0]*p.compute_var(P::V2)) );
  }

  virtual void get_state( Solver::Physics& p, RealVector& state)
  {
    typedef Euler::Physics P;

    state[0] = p.var(P::rho);
    state[1] = p.var(P::rho)*p.var(P::Vx);
    state[2] = p.var(P::rho)*p.var(P::Vy);
    state[3] = p.var(P::rho)*p.compute_var(P::E);
  }


  virtual void compute_flux( Solver::Physics& p,
                            const RealVector& normal,
                            RealVector& flux)
  {
    typedef Euler::Physics P;
    const Real un = p.var(P::Vx) * normal[XX] + p.var(P::Vy) * normal[YY];
    flux[0] = p.var(P::rho) * un;
    flux[1] = p.var(P::rho) * p.var(P::Vx) * un + p.var(P::p)*normal[XX];
    flux[2] = p.var(P::rho) * p.var(P::Vy) * un + p.var(P::p)*normal[YY];
    flux[3] = (p.var(P::rho) * p.compute_var(P::E) + p.var(P::p)) * un;
  }

  virtual void compute_fluxjacobian_right_eigenvectors( Solver::Physics& p,
                                                        const RealVector& normal,
                                                        RealMatrix& rv)
  {
    typedef Euler::Physics P;

    const Real r=p.var(P::rho);
    const Real u=p.var(P::Vx);
    const Real v=p.var(P::Vy);
    const Real h=p.compute_var(P::H);
    const Real a=p.compute_var(P::a);
    const Real nx=normal[XX];
    const Real ny=normal[YY];
    const Real un=u*nx+v*ny;

    rv <<
          1.,             0.,             0.5*r/a,             0.5*r/a,
          u,              r*ny,           0.5*r/a*(u+a*nx),    0.5*r/a*(u-a*nx),
          v,              -r*nx,          0.5*r/a*(v+a*ny),    0.5*r/a*(v-a*ny),
          0.5*(u*u+v*v),  r*(u*ny-v*nx),  0.5*r/a*(h+a*un),    0.5*r/a*(h-a*un);
  }

  virtual void compute_fluxjacobian_left_eigenvectors( Solver::Physics& p,
                                                       const RealVector& normal,
                                                       RealMatrix& lv)
  {
    typedef Euler::Physics P;

    const Real gm1=p.var(P::gamma_minus_1);
    const Real r=p.var(P::rho);
    const Real u=p.var(P::Vx);
    const Real v=p.var(P::Vy);
    const Real a=p.compute_var(P::a);
    const Real nx=normal[XX];
    const Real ny=normal[YY];
    const Real un=u*nx+v*ny;

    lv <<
          1.-0.5*gm1*(u*u+v*v)/(a*a),            gm1*u/(a*a),           gm1*v/(a*a),        -gm1/(a*a),
          1./r*(v*nx-u*ny),                        1./r*ny,                -1./r*nx,               0.,
          a/r*(0.5*gm1*(u*u+v*v)/(a*a)-un/a),    1./r*(nx-gm1*u/a),     1./r*(ny-gm1*v/a),   gm1/(r*a),
          a/r*(0.5*gm1*(u*u+v*v)/(a*a)+un/a),   -1./r*(nx+gm1*u/a),    -1./r*(ny+gm1*v/a),   gm1/(r*a);
    }

  virtual void compute_fluxjacobian_eigenvalues( Solver::Physics& p,
                                                 const RealVector& normal,
                                                 RealVector& ev)
  {
    typedef Euler::Physics P;

    const Real un = p.var(P::Vx) * normal[XX] + p.var(P::Vy) * normal[YY];
    const Real a = p.compute_var(P::a);
    ev <<   un,
            un,
            un+a,
            un-a;
  }

  virtual Real max_abs_eigen_value ( Solver::Physics& p, const RealVector& normal )
  {
    typedef Euler::Physics P;

    const Real un = p.var(P::Vx) * normal[XX] + p.var(P::Vy) * normal[YY];
    const Real a = p.compute_var(P::a);

    return std::abs(un) + a;
  }

  virtual void linearize( std::vector<Solver::Physics>& states, Solver::Physics& p )
  {
    throw Common::NotImplemented(FromHere(),"");
  }

}; // Cons2D

////////////////////////////////////////////////////////////////////////////////

} // Euler
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Euler_Cons2D_hpp
