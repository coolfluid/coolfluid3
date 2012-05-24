// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ShapeFunction_hpp
#define cf3_mesh_ShapeFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// This class represents the API to a ShapeFunction.
/// An object of this class is typically owned by a Space object,
/// which in turn is owned by a Entities object describing the element.
/// A shape function only exists in local coordinates. It has only a
/// dimensionality, and not a dimension.
/// @author Willem Deconinck
class Mesh_API ShapeFunction : public common::Component {

public: // typedefs

  
  

public: // functions

  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  /// Default constructor without arguments
  ShapeFunction( const std::string& name ) : common::Component(name) {}

  /// Default destructor
  virtual ~ShapeFunction() {}

  /// Type name: ShapeFunction
  static std::string type_name() { return "ShapeFunction"; }

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

  /// @return Number of faces of the shape function
  virtual Uint nb_faces() const = 0;

  /// @return Order of the shape function
  virtual Uint order() const = 0;

  /// @return dimensionality (e.g. shell in 3D world: dimensionality = 2)
  virtual Uint dimensionality() const = 0;

  /// @return a matrix with all local coordinates where the shape function is defined
  virtual const RealMatrix& local_coordinates() const = 0;

  /// @return a matrix with all face normals for every face as rows in local coordinates
  virtual const RealMatrix& face_normals() const = 0;

  //@}

  /// @name Computation functions
  //  ---------------------------
  //@{

  /// @brief Compute the shape function values in the given local coordinate
  /// @param [in] local_coordinate   local coordinate (size = dimensionality x 1)
  /// @return shape function values (size = 1 x nb_nodes)
  virtual RealRowVector value(const RealVector& local_coordinate) const = 0;

  /// @brief Compute the shape function values in the given local coordinate
  /// @param [in]  local_coordinate   local coordinate (size = dimensionality x 1)
  /// @param [out] value              computed value (size = 1 x nb_nodes)
  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const = 0;

  /// @brief Compute the shape function gradient in the given local coordinate
  /// @param [in] local_coordinate   local coordinate (size = dimensionality x 1)
  /// @return shape function gradient (size = dimensionality x nb_nodes)
  virtual RealMatrix gradient(const RealVector& local_coordinate) const = 0;

  /// @brief Compute the shape function values in the given local coordinate
  /// @param [in]  local_coordinate   local coordinate (size = dimensionality x 1)
  /// @param [out] gradient           computed gradient (size = dimensionality x nb_nodes)
  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const = 0;

  //@}

}; // ShapeFunction

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ShapeFunction_hpp
