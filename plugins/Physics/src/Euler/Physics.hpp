// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Euler_Physics_hpp
#define CF_Euler_Physics_hpp
#include <boost/assign/list_of.hpp>

#include "Solver/Physics.hpp"
#include "Euler/LibEuler.hpp"
#include "Math/LibMath.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Euler {

////////////////////////////////////////////////////////////////////////////////

/// @brief Base class to interface the physics
/// @author Willem Deconinck
class Euler_API Physics : public Solver::Physics {

public: // functions

  enum Vars {gamma=0, gamma_minus_1=1, R=2, rho=3, Vx=4, Vy=5, Vz=6, p=7, E=8, H=9, a=10, a2=11, M=12, T=13, V=14, V2=15};

  /// Contructor
  Physics() : Solver::Physics()
  {
    using namespace boost;
    using namespace boost::assign;

    resize(16);

    set_compute_function(gamma_minus_1, bind(&Physics::gamma_min_1,this));
    var_deps(gamma_minus_1) = list_of(gamma);

    set_compute_function(V2, bind(&Physics::velocity_magnitude_square,this));
    var_deps(V2) = list_of(Vx)(Vy)(Vz);

    set_compute_function(V, bind(&Physics::velocity_magnitude,this));
    var_deps(V) = list_of(V2);

    set_compute_function(E, bind(&Physics::energy,this));
    var_deps(E) = list_of(gamma_minus_1)(rho)(V2)(p);

    set_compute_function(H, bind(&Physics::enthalpy,this));
    var_deps(H) = list_of(rho)(E)(p);

    set_compute_function(T, bind(&Physics::temperature,this));
    var_deps(T) = list_of(rho)(R)(p);

    set_compute_function(a2, bind(&Physics::sound_speed_square,this));
    var_deps(a2) = list_of(gamma)(R)(p);

    set_compute_function(a, bind(&Physics::sound_speed,this));
    var_deps(a) = list_of(a2);

    set_compute_function(M, bind(&Physics::mach_number,this));
    var_deps(M) = list_of(V)(a);

  }

  Real gamma_min_1()
  {
    return var(gamma)-1.;
  }

  Real energy()
  {
    return var(p)/(var(rho)*var(gamma_minus_1)) + 0.5*var(V2);
  }

  Real enthalpy()
  {
    return var(E) + var(p)/var(rho);
  }

  Real temperature()
  {
    return var(p)/(var(rho)*var(R));
  }

  Real sound_speed_square()
  {
    return var(gamma)*var(p)/var(rho);
  }

  Real sound_speed()
  {
    return std::sqrt(var(a2));
  }

  Real velocity_magnitude_square()
  {
    return var(Vx)*var(Vx)+var(Vy)*var(Vy)+var(Vz)*var(Vz);
  }

  Real velocity_magnitude()
  {
    return std::sqrt(var(V2));
  }

  Real mach_number()
  {
    return var(V)/var(a);
  }

}; // Physics

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Euler_Physics_hpp
