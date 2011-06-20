// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Euler_Roe1D_hpp
#define CF_Euler_Roe1D_hpp

#include "Solver/State.hpp"
#include "Euler/Physics.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Euler {

////////////////////////////////////////////////////////////////////////////////

/// @brief Base class to interface the Roe1D
/// @author Willem Deconinck
class Euler_API Roe1D : public Solver::State {

public: // typedefs

  typedef boost::shared_ptr<Roe1D> Ptr;
  typedef boost::shared_ptr<Roe1D const> ConstPtr;

public: // functions

  /// Contructor
  Roe1D( const std::string& name = type_name() ) : Solver::State(name)
  {
    m_var_names.resize(3);
    m_var_names[0]="rho";
    m_var_names[1]="Vx";
    m_var_names[2]="h";
  }

  /// Virtual destructor
  virtual ~Roe1D() {}

  /// Gets the Class name
  static std::string type_name() { return "Roe1D"; }

  virtual Solver::Physics create_physics()
  {
    Euler::Physics p;
    return static_cast<Solver::Physics>(p);
  }

  virtual Uint size() { return 3; }

  virtual void set_state( const RealVector& state, Solver::Physics& p)
  {
    typedef Euler::Physics P;
    p.init();
    p.set_var(P::gamma, 1.4);
    p.set_var(P::R, 286.9);
    p.set_var(P::rho, state[0]);
    p.set_var(P::Vx,  state[1]);
    p.set_var(P::Vy,  0.);
    p.set_var(P::Vz,  0.);
    p.set_var(P::H ,  state[2]);
    p.set_var(P::E ,  1./p.var(P::gamma) * (p.var(P::H) + 0.5/(p.var(P::rho))*(p.var(P::gamma_minus_1)*p.var(P::V2))) );
    p.set_var(P::p ,  p.var(P::gamma_minus_1)*p.var(P::E)/p.var(P::rho)-0.5*p.var(P::rho)*p.var(P::V2));
  }

  virtual void get_state( Solver::Physics& p, RealVector& state)
  {
    typedef Euler::Physics P;

    state[0] = p.var(P::rho);
    state[1] = p.var(P::Vx);
    state[2] = p.var(P::H);
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

  virtual Real max_eigen_value ( Solver::Physics& p, const RealVector& normal )
  {
    throw Common::NotImplemented(FromHere(),"");
  }

  virtual void linearize( std::vector<Solver::Physics>& states, Solver::Physics& p )
  {
    RealVector avg_state(3);
    RealVector dummy_state(3);
    for (Uint i=0; i<states.size(); ++i)
    {
      get_state(states[i],dummy_state);
      avg_state += dummy_state;
    }
    avg_state /= static_cast<Real>(states.size());

    set_state(avg_state,p);
  }

}; // Roe1D

////////////////////////////////////////////////////////////////////////////////

} // Euler
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Euler_Roe1D_hpp
