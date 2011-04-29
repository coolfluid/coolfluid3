// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Line3DLagrangeP1_hpp
#define CF_Mesh_SF_Line3DLagrangeP1_hpp

#include "Mesh/SF/Line.hpp"
#include "Mesh/SF/SFLineLagrangeP1.hpp"
#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// line element.
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct MESH_SF_API Line3DLagrangeP1  : public Line<DIM_3D,SFLineLagrangeP1> {

  /// Constructor
  Line3DLagrangeP1(const std::string& name = type_name());

  /// The type name
  static std::string type_name() { return "Line3DLagrangeP1"; }

  /// The name of the builder
  virtual std::string builder_name() const { return LibSF::library_namespace()+"."+type_name(); }

  /// Compute Mapped Coordinates
  /// @param coord contains the coordinates to be mapped
  /// @param nodes contains the nodes
  /// @param mapped_coord Store the output mapped coordinates
  static void mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mapped_coord);

  /// Compute the Jacobian matrix
  /// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix (dimensionality x dimension)
  static void jacobian(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result);

  /// Volume of the cell
  static Real volume(const NodeMatrixT& nodes);

  /// Area of the cell
  static Real area(const NodeMatrixT& nodes);

  /// length of the cell
  static Real length(const NodeMatrixT& nodes);

  /// compute volume using given coordinates.
  virtual Real compute_volume(const NodesT& coord) const;

  /// compute area using given coordinates.
  /// @note area for a line in 3D is defined as 0
  virtual Real compute_area(const NodesT& coord) const;

  /// compute normal using given coordinates.
  virtual void compute_normal(const NodesT& coord, RealVector& normal) const;

  /// Compute the centroid
  /// @param coord node coordinates of the line
  /// @param centroid the computed centroid of the line
  virtual void compute_centroid(const NodesT& coord, RealVector& centroid ) const;

  /// Check if a coordinate is inside the element
  /// @param coord  A coordinate
  /// @param nodes  The coordinates of the nodes of the element
  /// @return true if the coordinate is inside the element
  virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;

  static const CF::Mesh::ElementType::FaceConnectivity& faces();
  virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
  virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

};

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF

#endif /* CF_Mesh_SF_Line3DLagrangeP1 */
