// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Hexa3DLagrangeP1_hpp
#define CF_Mesh_SF_Hexa3DLagrangeP1_hpp

#include <Eigen/Dense>

#include "Math/Functions.hpp"

#include "Mesh/Hexa3D.hpp"

#include "Mesh/SF/LibSF.hpp"
#include "Mesh/SF/SFHexaLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (bilinear)
/// quadrilateral element.
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
/// @author Willem Deconinck
struct MESH_SF_API Hexa3DLagrangeP1  : public Hexa3D {

public:

  Hexa3DLagrangeP1(const std::string& name = type_name());

  static std::string type_name() { return "Hexa3DLagrangeP1"; }

  virtual std::string builder_name() const { return LibSF::library_namespace()+"."+type_name(); }

  /// Number of nodes
  static const Uint nb_nodes = 8;

  /// Order of the shape function
  static const Uint order = 1;

  /// Types for the matrices used
  typedef Eigen::Matrix<Real, dimension, 1>              CoordsT;
  typedef Eigen::Matrix<Real, dimensionality, 1>         MappedCoordsT;
  typedef Eigen::Matrix<Real, nb_nodes, dimension>       NodeMatrixT;
  typedef Eigen::Matrix<Real, 1, nb_nodes>               ShapeFunctionsT;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes>  MappedGradientT;
  typedef Eigen::Matrix<Real, dimensionality, dimension> JacobianT;

  /// Shape function reference
  virtual const ShapeFunction& shape_function() const
  {
    const static SFHexaLagrangeP1 shape_function_obj;
    return shape_function_obj;
  }

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  /// @param mappedCoord The mapped coordinates
  /// @param shapeFunc Vector storing the result
  static void shape_function_value(const MappedCoordsT& mapped_coord, ShapeFunctionsT& shape_func);

  /// Compute Mapped Coordinates
  /// @param coord contains the coordinates to be mapped
  /// @param nodes contains the nodes
  /// @param mappedCoord Store the output mapped coordinates
  static void mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mapped_coord);

  /// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
  /// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
  /// coordinates.
  /// @param mapped_coord The mapped coordinates where the gradient should be calculated
  /// @param result Storage for the resulting gradient matrix
  static void shape_function_gradient(const MappedCoordsT& mapped_coord, MappedGradientT& result);

  /// Compute the jacobian determinant at the given mapped coordinates
  static Real jacobian_determinant(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes);

  /// Compute the Jacobian matrix
  /// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  static void jacobian(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result);
  /// Compute the adjoint of Jacobian matrix
  /// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting adjoint
  static void jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result);

  /// Volume of the cell
  static Real volume(const NodeMatrixT& nodes);

  //template<typename NodesT>
  static bool in_element(const CoordsT& coord, const NodeMatrixT& nodes);

  /// Indices for the faces in each direction
  enum FaceNumbering { ZNEG, ZPOS, YNEG, XPOS, YPOS, XNEG};

  static const FaceConnectivity& faces();

  virtual Real compute_volume(const NodesT& coord) const;
  virtual void compute_centroid(const NodesT& coord , RealVector& centroid) const;
  virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;
  virtual const FaceConnectivity& face_connectivity() const;
  virtual const ElementType& face_type(const Uint face) const;

private:

  /// @return if coordinate is oriented towards the inside of the element from the point of view from a given face
  /// @param coord [in]  coordinates
  /// @param nodes [in]  the nodes defining the element
  /// @param face  [in]  the face number fo the element
  static bool is_orientation_inside(const CoordsT& coord, const NodeMatrixT& nodes, const Uint face);

};

} // SF
} // Mesh
} // CF

#endif /* CF_Mesh_SF_Hexa3DLagrangeP1 */
