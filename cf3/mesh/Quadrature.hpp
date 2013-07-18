// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Quadrature_hpp
#define cf3_mesh_Quadrature_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// This class represents the API to a Quadrature.
/// A quadrature only exists in local coordinates. It has only a
/// dimensionality, and not a dimension.
/// @author Willem Deconinck
class Mesh_API Quadrature : public common::Component {

public: // functions

  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  /// Default constructor without arguments
  Quadrature( const std::string& name ) : common::Component(name) {}

  /// Default destructor
  virtual ~Quadrature() {}

  /// Type name: Quadrature
  static std::string type_name() { return "Quadrature"; }

  //@}

  /// @name Accessor functions
  //  ------------------------
  //@{

  /// @return shape as string
  std::string shape_name() const { return mesh::GeoShape::Convert::instance().to_str( shape() ); }

  /// @return shape as enumerator
  virtual GeoShape::Type shape() const = 0;

  /// @return number of nodes
  virtual Uint nb_nodes() const = 0;

  /// @return Order of the quadrature (exact integration of polynomials of this order)
  virtual Uint order() const = 0;

  /// @return dimensionality (e.g. shell in 3D world: dimensionality = 2)
  virtual Uint dimensionality() const = 0;

  /// @return a matrix with all local coordinates where the quadrature weights are defined
  virtual const RealMatrix& local_coordinates() const = 0;

  /// @return a matrix with all local coordinates where the quadrature weights are defined
  virtual const RealRowVector& weights() const = 0;

}; // Quadrature

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Quadrature_hpp
