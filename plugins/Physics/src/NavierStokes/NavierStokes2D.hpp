// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_NavierStokes_NavierStokes2D_hpp
#define CF_NavierStokes_NavierStokes2D_hpp

#include "Physics/PhysModel.hpp"

#include "LibNavierStokes.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace NavierStokes {

///////////////////////////////////////////////////////////////////////////////////////

class NavierStokes_API NavierStokes2D : public Physics::PhysModel {

public: // functions

  enum { _ndim = 2 };
  enum { _neqs = 4 };

  /// Constructor
  NavierStokes2D ( const std::string& name );

  /// Destructor
  virtual ~NavierStokes2D();

  /// Get the class name
  static std::string type_name () { return "NavierStokes2D"; }

  /// physical properties
  struct Properties : public Physics::Properties
  {
    Real gamma;               /// specific heat ratio
    Real gamma_minus_1;       /// specific heat ratio minus one, very commonly used
    Real R;                   /// gas constant

    Real rho;                 /// density
    Real inv_rho;             /// inverse of density, very commonly used
    Real rhou;                /// rho.u
    Real rhov;                /// rho.v
    Real rhoE;                /// rho.E

    Real u;                   /// velocity along XX
    Real v;                   /// velocity along YY
    Real uuvv;                /// u^2 + v^2

    Real H;                   /// specific enthalpy
    Real a2;                  /// square of speed of sound, very commonly used
    Real a;                   /// speed of sound
    Real P;                   /// pressure
    Real T;                   /// temperature
    Real E;                   /// specific internal energy
    Real half_gm1_v2;         /// 1/2.(g-1).(u^2+v^2), very commonly used
    Real Ma;                  /// mach number
  };

  /// @name INTERFACE
  //@{

  /// @returns the dimensionality of this model
  virtual Uint ndim() { return (Uint) _ndim; }
  /// @returns the number of equations
  virtual Uint neqs() { return (Uint) _neqs; }
  /// @return the physical model type
  virtual std::string type() const { return type_name(); }
  /// create a physical properties
  virtual Physics::Properties* create_properties()
  {
    return new NavierStokes2D::Properties();
  }
  /// create a variables description
  virtual Physics::Variables* create_variables( const std::string& name );

  //@} END INTERFACE

}; // NavierStokes2D

////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_NavierStokes_NavierStokes2D_hpp
