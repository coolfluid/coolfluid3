// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Point3DLagrangeP0_hpp
#define CF_Mesh_SF_Point3DLagrangeP0_hpp


#include "Mesh/SF/Point.hpp"
#include "Mesh/SF/SFPointLagrangeP0.hpp"
#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P0 (Pointar)
/// Point element.
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct MESH_SF_API Point3DLagrangeP0  : public Point<DIM_3D,SFPointLagrangeP0> {

  /// Constructor
  Point3DLagrangeP0(const std::string& name = type_name());

  /// The type name
  static std::string type_name() { return "Point3DLagrangeP0"; }

  /// The name of the builder
  virtual std::string builder_name() const { return LibSF::library_namespace()+"."+type_name(); }

  /// Compute Mapped Coordinates
  /// @param coord contains the coordinates to be mapped
  /// @param nodes contains the nodes
  /// @param mapped_coord Store the output mapped coordinates
  static void mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mapped_coord);

  /// Compute the jacobian determinant at the given mapped coordinates
  static Real jacobian_determinant(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes);

  /// Compute the Jacobian matrix
  /// @param mapped_coord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  static void jacobian(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result);

  /// Compute the adjoint of Jacobian matrix
  /// @param mapped_coord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting adjoint
  static void jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result);

  /// Volume of the cell
  static Real volume(const NodeMatrixT& nodes);

  /// Area of the cell
  static Real area(const NodeMatrixT& nodes);

  /// Normal vector to the surface. Length equals the jacobian norm.
  /// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  static void normal(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, CoordsT& result);

  /// compute volume using given coordinates.
  /// @note volume for a point in 3D is defined as 0
  virtual Real compute_volume(const NodesT& coord) const;

  /// compute area using given coordinates.
  /// @note area for a point in 3D is defined as 0
  virtual Real compute_area(const NodesT& coord) const;

  /// compute normal using given coordinates.
  /// @note normal for a point in 3D is undefined
  /// @throws Common::IllegalCall
  virtual void compute_normal(const NodesT& coord, RealVector& normal) const;

  static const CF::Mesh::ElementType::FaceConnectivity& faces();
  virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
  virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

};

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF

#endif //CF_Mesh_SF_Point3DLagrangeP0
