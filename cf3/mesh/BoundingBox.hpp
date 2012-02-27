// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_BoundingBox_hpp
#define cf3_mesh_BoundingBox_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"
#include "mesh/LibMesh.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  
  class Region;
  class Mesh;
  class Field;

//////////////////////////////////////////////////////////////////////////////

/// @brief Bounding box defined by minimum and maximum coordinates
/// @author Willem Deconinck
class Mesh_API BoundingBox : public common::Component
{
public: // functions

  /// Gets the Class name
  static std::string type_name() { return "BoundingBox"; }

  /// constructor using RealVector
  BoundingBox(const std::string& name);

  /// @brief Define bounding box with RealVectors
  void define(const RealVector& min, const RealVector& max);

  /// @brief Define bounding box with std::vector<Real>
  void define(const std::vector<Real>& min, const std::vector<Real>& max);

  /// @brief Build bounding box with a mesh
  /// @param [in] region  Region to create bounding box for
  void build(const Region& region);

  /// @brief Build bounding box with a region
  /// @param [in] mesh  Mesh to create bounding box for
  void build(const Mesh& mesh);

  /// @brief Build bounding box with a coordinates
  /// @param [in] coordinates  Coordinates to create bounding box for
  void build(const Field& coordinates);

  /// @brief Expand bounding box to encompass all processors
  /// @note This function must be called on all processors
  void make_global();

public: //functions

  /// @brief Check if coordinate falls inside the bounding box
  bool contains(const RealVector& coordinate) const;

  /// @brief minimum coordinates, defining one corner of the bounding box
  const RealVector& min() const { return m_bounding_min; }

  /// @brief maximum coordinates, defining one corner of the bounding box  
  const RealVector& max() const { return m_bounding_max; }  

  /// @brief minimum coordinates, defining one corner of the bounding box
  RealVector& min() { return m_bounding_min; }

  /// @brief maximum coordinates, defining one corner of the bounding box  
  RealVector& max() { return m_bounding_max; }  
  
  /// @brief dimension of the bounding box
  Uint dim() const { return m_bounding_min.size(); }
  
private: // data

  RealVector m_bounding_min; ///< minimum coordinates
  RealVector m_bounding_max; ///< maximum coordinates

}; // end BoundingBox

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_BoundingBox_hpp
