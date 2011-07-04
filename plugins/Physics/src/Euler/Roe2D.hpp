// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Euler_Roe2D_hpp
#define CF_Euler_Roe2D_hpp

#include "Solver/State.hpp"
#include "Euler/Physics.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Euler {

////////////////////////////////////////////////////////////////////////////////

/// @brief Base class to interface the Roe2D
/// @author Willem Deconinck
class Euler_API Roe2D : public Solver::State {

public: // typedefs

  typedef boost::shared_ptr<Roe2D> Ptr;
  typedef boost::shared_ptr<Roe2D const> ConstPtr;

public: // functions

  /// Contructor
  Roe2D( const std::string& name = type_name() ) : Solver::State(name)
  {
    m_var_names.resize(4);
    m_var_names[0]="rho";
    m_var_names[1]="Vx";
    m_var_names[2]="Vy";
    m_var_names[3]="h";
  }

  /// Virtual destructor
  virtual ~Roe2D() {}

  /// Gets the Class name
  static std::string type_name() { return "Roe2D"; }

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
    p.set_var(P::Vx,  state[1]);
    p.set_var(P::Vy,  state[2]);
    p.set_var(P::Vz,  0.);
    p.set_var(P::H ,  state[3]);
    p.set_var(P::E ,  1./p.var(P::gamma) * (p.var(P::H) + 0.5/(p.var(P::rho))*(p.compute_var(P::gamma_minus_1)*p.compute_var(P::V2))) );
    p.set_var(P::p ,  p.var(P::gamma_minus_1)*(p.var(P::E)*p.var(P::rho)-0.5/p.var(P::rho)*p.var(P::V2)) );
  }

  virtual void get_state( Solver::Physics& p, RealVector& state)
  {
    typedef Euler::Physics P;

    state[0] = p.var(P::rho);
    state[1] = p.var(P::Vx);
    state[2] = p.var(P::Vy);
    state[3] = p.compute_var(P::H);
  }


  virtual void compute_flux( Solver::Physics& p,
                            const RealVector& normal,
                            RealVector& flux)
  {
    throw Common::NotImplemented(FromHere(),"");
  }

  virtual void compute_fluxjacobian_right_eigenvectors( Solver::Physics& p,
                                                        const RealVector& normal,
                                                        RealMatrix& rv)
  {
    throw Common::NotImplemented(FromHere(),"");
  }

  virtual void compute_fluxjacobian_left_eigenvectors( Solver::Physics& p,
                                                       const RealVector& normal,
                                                       RealMatrix& lv)
  {
    throw Common::NotImplemented(FromHere(),"");
  }

  virtual void compute_fluxjacobian_eigenvalues( Solver::Physics& p,
                                                 const RealVector& normal,
                                                 RealVector& ev)
  {
    throw Common::NotImplemented(FromHere(),"");
  }

  virtual Real max_abs_eigen_value ( Solver::Physics& p, const RealVector& normal )
  {
    throw Common::NotImplemented(FromHere(),"");
  }

  virtual void linearize( std::vector<boost::shared_ptr<Solver::Physics> >& states, Solver::Physics& p )
  {
    typedef Euler::Physics P;

    enum Side {LEFT=0,RIGHT=1};
    const Real sqrt_rho_L = sqrt(states[LEFT ]->var(P::rho));
    const Real sqrt_rho_R = sqrt(states[RIGHT]->var(P::rho));
    p.init();
    // compute roe average quantities
    p.set_var(P::rho , sqrt_rho_L * sqrt_rho_R);
    p.set_var(P::Vx  , (sqrt_rho_L*states[LEFT]->var(P::Vx) + sqrt_rho_R*states[RIGHT]->var(P::Vx)) / (sqrt_rho_L + sqrt_rho_R) );
    p.set_var(P::Vy  , (sqrt_rho_L*states[LEFT]->var(P::Vy) + sqrt_rho_R*states[RIGHT]->var(P::Vy)) / (sqrt_rho_L + sqrt_rho_R) );
    p.set_var(P::H   , (sqrt_rho_L*states[LEFT]->compute_var(P::H)  + sqrt_rho_R*states[RIGHT]->compute_var(P::H) ) / (sqrt_rho_L + sqrt_rho_R) );

    p.set_var(P::gamma , 1.4);
    p.set_var(P::R, 286.9);
    p.set_var(P::Vz  , 0.);
    p.set_var(P::p , p.compute_var(P::gamma_minus_1)/p.var(P::gamma)*p.var(P::rho)*(p.var(P::H)-0.5*p.compute_var(P::V2)) );

    if ( p.compute_var(P::a2) < 0.0 )
    {
      std::string msg = "Euler::Roe2D::linearize() : a2 < 0 => a2 = " + Common::to_str(p.var(P::a2));
      throw Common::BadValue (FromHere(),msg);
    }

  }

}; // Roe2D

////////////////////////////////////////////////////////////////////////////////

} // Euler
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Euler_Roe2D_hpp
