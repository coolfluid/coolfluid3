// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Physics_NavierStokes_NavierStokes2D_hpp
#define CF_Physics_NavierStokes_NavierStokes2D_hpp

#include "Physics/PhysModel.hpp"

#include "Math/Defs.hpp"
#include "Math/MatrixTypes.hpp"

#include "LibNavierStokes.hpp"

namespace CF {
namespace Physics {
namespace NavierStokes {

//////////////////////////////////////////////////////////////////////////////////////////////

class NavierStokes_API NavierStokes2D : public Physics::PhysModel {

public: // typedefs

  enum { _ndim = 2 }; ///< number of dimensions
  enum { _neqs = 4 }; ///< number of independent variables or equations

  typedef Eigen::Matrix<Real, _ndim, 1>    GeoV;  ///< type of geometry coordinates vector
  typedef Eigen::Matrix<Real, _neqs, 1>    SolV;  ///< type of solution variables vector
  typedef Eigen::Matrix<Real, _neqs,_ndim> SolM;  ///< type of solution gradient matrix

public: // functions

  /// Constructor
  NavierStokes2D ( const std::string& name );

  /// Destructor
  virtual ~NavierStokes2D();

  /// Get the class name
  static std::string type_name () { return "NavierStokes2D"; }

  /// physical properties
  struct Properties : public Physics::Properties
  {

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

    GeoV coords;       ///< position in domain
    SolV vars;         ///< independent variables with positions described in Variables
    SolM grad_vars;    ///< gradient of independent variables

    /// @name Gas constants, for now hardcoded in .cpp file
    //@{
    static const Real gamma;            ///< specific heat ratio
    static const Real R;                ///< gas constant
    static const Real gamma_minus_1;    ///< specific heat ratio minus one, very commonly used
    //@}

    Real rho;                 ///< density
    Real rhou;                ///< rho.u
    Real rhov;                ///< rho.v
    Real rhoE;                ///< rho.E

    Real inv_rho;             ///< inverse of density, very commonly used

    Real u;                   ///< velocity along XX
    Real v;                   ///< velocity along YY
    Real uuvv;                ///< u^2 + v^2

    Real H;                   ///< specific enthalpy
    Real a2;                  ///< square of speed of sound, very commonly used
    Real a;                   ///< speed of sound
    Real P;                   ///< pressure
    Real T;                   ///< temperature
    Real E;                   ///< specific internal energy
    Real half_gm1_v2;         ///< 1/2.(g-1).(u^2+v^2), very commonly used
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
  virtual std::auto_ptr<Physics::Properties> create_properties()
  {
    return std::auto_ptr<Physics::Properties>( new NavierStokes2D::Properties() );
  }

  /// Create a Variables component
  /// @param type is the name of the Variables
  /// @post the component will be a sub-component of this model but maybe be moved away
  /// @throws ValueNotFound if the type does not match a variable type this model supports
  virtual boost::shared_ptr< Physics::Variables > create_variables( const std::string type, const std::string name );
  //@} END INTERFACE

}; // NavierStokes2D

//////////////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // CF

#endif // CF_Physics_NavierStokes_NavierStokes2D_hpp
