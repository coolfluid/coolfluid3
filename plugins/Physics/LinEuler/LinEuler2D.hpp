// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Physics_LinEuler_LinEuler2D_hpp
#define CF_Physics_LinEuler_LinEuler2D_hpp

#include "Physics/PhysModel.hpp"

#include "Math/Defs.hpp"
#include "Math/MatrixTypes.hpp"

#include "LibLinEuler.hpp"

namespace CF {
namespace Physics {
namespace LinEuler {

//////////////////////////////////////////////////////////////////////////////////////////////

class LinEuler_API LinEuler2D : public Physics::PhysModel {

public: // typedefs

  enum { _ndim = 2 }; ///< number of dimensions
  enum { _neqs = 4 }; ///< number of independent variables or equations

  typedef Eigen::Matrix<Real, _ndim, 1>    GeoV;  ///< type of geometry coordinates vector
  typedef Eigen::Matrix<Real, _neqs, 1>    SolV;  ///< type of solution variables vector
  typedef Eigen::Matrix<Real, _neqs,_ndim> SolM;  ///< type of solution gradient matrix

public: // functions

  /// Constructor
  LinEuler2D ( const std::string& name );

  /// Destructor
  virtual ~LinEuler2D();

  /// Get the class name
  static std::string type_name () { return "LinEuler2D"; }

  /// physical properties
  struct Properties : public Physics::Properties
  {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

    GeoV coords;       ///< position in domain
    SolV vars;         ///< independent variables with positions described in Variables
    SolM grad_vars;    ///< gradient of independent variables

    // HARD CODED FOR NOW, in .cpp file
    static const Real gamma;               ///< specific heat ratio
    static const GeoV u0;                  ///< background (mean) velocity
    static const Real rho0;                ///< background (mean) density
    static const Real P0;                  ///< background (mean) pressure
    static const Real c;                   ///< speed of sound based on mean quantities
    static const Real inv_c;               ///< inverse of the speed of sound, very commonly used
    static const Real inv_rho0;            ///< inverse of referenceC density, very commonly used

    Real rho;                 ///< density
    Real rho0u;               ///< rho0.u
    Real rho0v;               ///< rho0.v
    Real p;                   ///< acoustic pressure
    Real u;                   ///< velocity along XX, rho0.u / rho0
    Real v;                   ///< velocity along YY, rho0.v / rho0
    Real H;                   ///< acoustic enthalpy
  };

  /// @name INTERFACE
  //@{

  /// @returns the dimensionality of this model
  virtual Uint ndim() const { return (Uint) _ndim; }
  /// @returns the number of equations
  virtual Uint neqs() const { return (Uint) _neqs; }
  /// @return the physical model generic type
  virtual std::string model_type() const { return "LinEuler"; }
  /// @return the physical model type
  virtual std::string type() const { return type_name(); }
  /// create a physical properties
  virtual std::auto_ptr<Physics::Properties> create_properties()
  {
    return std::auto_ptr<Physics::Properties>( new LinEuler2D::Properties() );
  }

  /// Create a Variables component
  /// @param type is the name of the Variables
  /// @post the component will be a sub-component of this model but maybe be moved away
  /// @throws ValueNotFound if the type does not match a variable type this model supports
  virtual boost::shared_ptr< Physics::Variables > create_variables( const std::string type, const std::string name );

  //@} END INTERFACE

}; // LinEuler2D

//////////////////////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // Physics
} // CF

#endif // CF_Physics_LinEuler_LinEuler2D_hpp
