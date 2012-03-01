// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_BoundingBox_hpp
#define cf3_mesh_BoundingBox_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/BoundingBox.hpp"
#include "mesh/LibMesh.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  
  class Region;
  class Mesh;
  class Field;

//////////////////////////////////////////////////////////////////////////////

/// @brief Wrapped math::BoundingBox, with extra functionality
/// @author Willem Deconinck
class Mesh_API BoundingBox : public common::Component, public math::BoundingBox
{
public: // functions

  /// Gets the Class name
  static std::string type_name() { return "BoundingBox"; }

  /// constructor using RealVector
  BoundingBox(const std::string& name);

  /// @brief Build bounding box with a mesh
  /// @param [in] region  Region to create bounding box for
  void build(const Region& region);

  /// @brief Build bounding box with a region
  /// @param [in] mesh  Mesh to create bounding box for
  void build(const Mesh& mesh);

  /// @brief Build bounding box with a coordinates
  /// @param [in] coordinates  Coordinates to create bounding box for
  void build(const Field& coordinates);

  /// @brief Update the properties
  void update_properties();

}; // end BoundingBox

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_BoundingBox_hpp
