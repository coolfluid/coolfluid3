// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_NavierStokes_NavierStokes3D_hpp
#define cf3_physics_NavierStokes_NavierStokes3D_hpp

#include "physics/PhysModel.hpp"

#include "math/Defs.hpp"
#include "math/MatrixTypes.hpp"

#include "LibNavierStokes.hpp"

namespace cf3 {
namespace physics {
namespace NavierStokes {

//////////////////////////////////////////////////////////////////////////////////////////////

class NavierStokes_API NavierStokes3D : public physics::PhysModel {

public: // typedefs

  enum { _ndim = 3 }; ///< number of dimensions
  enum { _neqs = 5 }; ///< number of independent variables or equations

  typedef Eigen::Matrix<Real, _ndim, 1>    GeoV;  ///< type of geometry coordinates vector
  typedef Eigen::Matrix<Real, _neqs, 1>    SolV;  ///< type of solution variables vector
  typedef Eigen::Matrix<Real, _neqs,_ndim> SolM;  ///< type of solution gradient matrix

public: // functions

  /// Constructor
  NavierStokes3D ( const std::string& name );

  /// Destructor
  virtual ~NavierStokes3D();

  /// Get the class name
  static std::string type_name () { return "NavierStokes3D"; }

  /// physical properties
  struct Properties : public physics::Properties
  {

    Properties()
    {
      gamma = 1.4;
      R = 287.05;
      gamma_minus_1 = gamma - 1.;
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

    GeoV coords;       ///< position in domain
    SolV vars;         ///< independent variables with positions described in Variables
    SolM grad_vars;    ///< gradient of independent variables

    /// @name Gas constants, configurable
    //@{
    Real gamma;            ///< specific heat ratio
    Real R;                ///< gas constant
    Real gamma_minus_1;    ///< specific heat ratio minus one, very commonly used
    //@}

    Real rho;                 ///< density
    Real rhou;                ///< rho.u
    Real rhov;                ///< rho.v
    Real rhow;                ///< rho.v
    Real rhoE;                ///< rho.E

    Real inv_rho;             ///< inverse of density, very commonly used

    Real u;                   ///< velocity along XX
    Real v;                   ///< velocity along YY
    Real w;                   ///< velocity along ZZ
    Real uuvvww;              ///< u^2 + v^2 + w^2

    Real H;                   ///< specific enthalpy
    Real a2;                  ///< square of speed of sound, very commonly used
    Real a;                   ///< speed of sound
    Real P;                   ///< pressure
    Real T;                   ///< temperature
    Real E;                   ///< specific internal energy
    Real half_gm1_v2;         ///< 1/2.(g-1).(u^2+v^2+w^2), very commonly used
    Real Ma;                  ///< mach number
  };

  /// @name INTERFACE
  //@{

  /// @returns the dimensionality of this model
  virtual Uint ndim() const { return (Uint) _ndim; }
  /// @returns the number of equations
  virtual Uint neqs() const { return (Uint) _neqs; }
  /// @return the physical model generic type
  virtual std::string model_type() const { return "NavierStokes"; }
  /// @return the physical model type
  virtual std::string type() const { return type_name(); }
  /// create a physical properties
  virtual std::auto_ptr<physics::Properties> create_properties()
  {
    std::auto_ptr<physics::Properties> props( new NavierStokes3D::Properties() );
    set_constants( static_cast<NavierStokes3D::Properties&>( *props ) );
    return props;
  }

  /// Create a Variables component
  /// @param type is the name of the Variables
  /// @post the component will be a sub-component of this model but maybe be moved away
  /// @throws ValueNotFound if the type does not match a variable type this model supports
  virtual boost::shared_ptr< physics::Variables > create_variables( const std::string type, const std::string name );

  //@} END INTERFACE

private:

  void set_constants(NavierStokes3D::Properties& props)
  {
    props.gamma = m_gamma;
    props.gamma_minus_1 = m_gamma - 1.;
    props.R = m_R;
  }

  Real m_gamma;
  Real m_R;

}; // NavierStokes3D

//////////////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // physics
} // cf3

#endif // cf3_physics_NavierStokes_NavierStokes3D_hpp
