// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Physics_Scalar_ScalarSys2D_hpp
#define CF_Physics_Scalar_ScalarSys2D_hpp

#include "Physics/PhysModel.hpp"

#include "Math/Defs.hpp"
#include "Math/MatrixTypes.hpp"

#include "LibScalar.hpp"

namespace CF {
namespace Physics {
namespace Scalar {

//////////////////////////////////////////////////////////////////////////////////////////////

class Scalar_API ScalarSys2D : public Physics::PhysModel {

public: // typedefs

  enum { _ndim = 2 }; ///< number of dimensions
  enum { _neqs = 2 }; ///< number of independent variables or equations

  typedef Eigen::Matrix<Real, _ndim, 1>    GeoV;  ///< type of geometry coordinates vector
  typedef Eigen::Matrix<Real, _neqs, 1>    SolV;  ///< type of solution variables vector
  typedef Eigen::Matrix<Real, _neqs,_ndim> SolM;  ///< type of solution gradient matrix

public: // functions

  /// Constructor
  ScalarSys2D ( const std::string& name );

  /// Destructor
  virtual ~ScalarSys2D();

  /// Get the class name
  static std::string type_name () { return "ScalarSys2D"; }

  /// physical properties
  struct Properties : public Physics::Properties
  {
    GeoV coords;       ///< position in domain
    SolV vars;         ///< independent variables with positions described in Variables
    SolM grad_vars;    ///< gradient of independent variables

    SolM v;            ///< advection speed per equation

    SolV mu;            ///< salar diffusion coefficient per equation

    Real u0;            ///< scalar variable
    Real u1;            ///< scalar variable
  };

  /// @name INTERFACE
  //@{

  /// @returns the dimensionality of this model
  virtual Uint ndim() const { return (Uint) _ndim; }
  /// @returns the number of equations
  virtual Uint neqs() const { return (Uint) _neqs; }
  /// @return the physical model generic type
  virtual std::string model_type() const { return "Scalar"; }
  /// @return the physical model type
  virtual std::string type() const { return type_name(); }
  /// create a physical properties
  virtual std::auto_ptr<Physics::Properties> create_properties()
  {
    return std::auto_ptr<Physics::Properties>( new ScalarSys2D::Properties() );
  }

  /// Create a Variables component
  /// @param type is the name of the Variables
  /// @post the component will be a sub-component of this model but maybe be moved away
  /// @throws ValueNotFound if the type does not match a variable type this model supports
  virtual boost::shared_ptr< Physics::Variables > create_variables( const std::string type, const std::string name );

  //@} END INTERFACE

}; // ScalarSys2D

//////////////////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // Physics
} // CF

#endif // CF_Physics_Scalar_ScalarSys2D_hpp
