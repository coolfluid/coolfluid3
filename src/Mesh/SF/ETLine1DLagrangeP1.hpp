// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_ETLine1DLagrangeP1_hpp
#define CF_Mesh_SF_ETLine1DLagrangeP1_hpp

#include "Math/MatrixTypes.hpp"

#include "Mesh/SF/Line.hpp"
#include "Mesh/SF/SFLineLagrangeP1.hpp"
#include "Mesh/SF/LibSF.hpp"
#include "Mesh/ElementData.hpp"

namespace CF {
namespace Mesh {
namespace SF {

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// line element.
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
struct MESH_SF_API ETLine1DLagrangeP1  : public Line<DIM_1D,SFLineLagrangeP1> {
  
  ETLine1DLagrangeP1(const std::string& name = type_name());

  static std::string type_name() { return "ETLine1DLagrangeP1"; }

  /// Compute Mapped Coordinates
  /// @param coord contains the coordinates to be mapped
  /// @param nodes contains the nodes
  /// @param mappedCoord Store the output mapped coordinates
  static void mapped_coordinates(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mappedCoord);

  /// Compute the jacobian determinant at the given mapped coordinates
  static Real jacobian_determinant(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes);

  /// Compute the Jacobian matrix
  /// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  static void jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result);

  /// Compute the adjoint of Jacobian matrix
  /// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting adjoint
  static void jacobian_adjoint(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result);

  /// Volume of the cell
  static Real volume(const NodeMatrixT& nodes); // inline because of multiple definition at link time
  
  static const CF::Mesh::ElementType2::FaceConnectivity& faces();
  
  virtual Real compute_volume(const NodesT& coord) const;
  virtual void compute_centroid(const NodesT& coord, RealVector& centroid ) const;
  virtual bool is_coord_in_element( const RealVector& coord, const NodesT& nodes) const;
  virtual const CF::Mesh::ElementType2::FaceConnectivity& face_connectivity() const;
  virtual const CF::Mesh::ElementType2& face_type(const CF::Uint face) const;

};

inline Real ETLine1DLagrangeP1::volume(const NodeMatrixT& nodes)
{
  return std::abs(nodes(1, XX) - nodes(0, XX));
}


} // SF
} // Mesh
} // CF

#endif /* CF_Mesh_SF_ETLine1DLagrangeP1 */
