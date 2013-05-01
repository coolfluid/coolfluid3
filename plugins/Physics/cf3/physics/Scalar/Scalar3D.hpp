// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_Scalar_Scalar3D_hpp
#define cf3_physics_Scalar_Scalar3D_hpp

#include "cf3/physics/PhysModel.hpp"

#include "math/Defs.hpp"
#include "math/MatrixTypes.hpp"

#include "LibScalar.hpp"

namespace cf3 {
namespace physics {
namespace Scalar {

//////////////////////////////////////////////////////////////////////////////////////////////

class Scalar_API Scalar3D : public physics::PhysModel {

public: // typedefs

  enum { _ndim = 3 }; ///< number of dimensions
  enum { _neqs = 1 }; ///< number of independent variables or equations

  typedef Eigen::Matrix<Real, _ndim, 1>    GeoV;  ///< type of geometry coordinates vector
  typedef Eigen::Matrix<Real, _neqs, 1>    SolV;  ///< type of solution variables vector
  typedef Eigen::Matrix<Real, _neqs,_ndim> SolM;  ///< type of solution gradient matrix

public: // functions

  /// Constructor
  Scalar3D ( const std::string& name );

  /// Destructor
  virtual ~Scalar3D();

  /// Get the class name
  static std::string type_name () { return "Scalar3D"; }

  /// physical properties
  struct Properties : public physics::Properties
  {

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

    GeoV coords;       ///< position in domain
    SolV vars;         ///< independent variables with positions described in Variables
    SolM grad_vars;    ///< gradient of independent variables

    GeoV v;            ///< advection speed

    Real mu;           ///< salar diffusion coefficient

    Real u;            ///< scalar variable
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
  virtual std::auto_ptr<physics::Properties> create_properties()
  {
    return std::auto_ptr<physics::Properties>( new Scalar3D::Properties() );
  }

  /// Create a Variables component
  /// @param type is the name of the Variables
  /// @post the component will be a sub-component of this model but maybe be moved away
  /// @throws ValueNotFound if the type does not match a variable type this model supports
  virtual boost::shared_ptr< physics::Variables > create_variables( const std::string type, const std::string name );

  //@} END INTERFACE

}; // Scalar3D

//////////////////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // physics
} // cf3

#endif // cf3_physics_Scalar_Scalar3D_hpp
