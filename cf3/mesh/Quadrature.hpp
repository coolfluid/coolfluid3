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
  GeoShape::Type shape() const { return m_shape; }

  /// @return number of nodes
  Uint nb_nodes() const { return m_nb_nodes; }

  /// @return Order of the quadrature (exact integration of polynomials of this order)
  Uint order() const { return m_order; }

  /// @return dimensionality (e.g. shell in 3D world: dimensionality = 2)
  Uint dimensionality() const { return m_dimensionality; }

  /// @return a matrix with all local coordinates where the quadrature weights are defined
  const RealMatrix& local_coordinates() const { return m_local_coordinates; }

  /// @return a matrix with all local coordinates where the quadrature weights are defined
  const RealRowVector& weights() const { return m_weights; }

  //@}

protected:

  GeoShape::Type m_shape;
  Uint m_nb_nodes;
  Uint m_order;
  Uint m_dimensionality;
  RealMatrix m_local_coordinates;
  RealRowVector m_weights;

}; // Quadrature

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Quadrature_hpp
