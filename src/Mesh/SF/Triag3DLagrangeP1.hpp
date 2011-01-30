// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Triag3DLagrangeP1_hpp
#define CF_Mesh_SF_Triag3DLagrangeP1_hpp

#include "Math/MatrixTypes.hpp"

#include "Mesh/Triag3D.hpp"

#include "Mesh/SF/Triag2DLagrangeP1.hpp"

#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (bilinear)
/// quadrilateral element that lives in 3D space
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
struct MESH_SF_API Triag3DLagrangeP1  : public Triag3D
{

  Triag3DLagrangeP1(const std::string& name = type_name());

  static std::string type_name() { return "Triag3DLagrangeP1"; }

  virtual std::string builder_name() const { return LibSF::library_namespace()+"."+type_name(); }

  /// Number of nodes
  static const Uint nb_nodes = 3;

  /// Order of the shape function
  static const Uint order = 1;
  
  /// Types for the matrices used
  typedef Eigen::Matrix<Real, dimension, 1> CoordsT;
  typedef Eigen::Matrix<Real, dimensionality, 1> MappedCoordsT;
  typedef Eigen::Matrix<Real, nb_nodes, dimension> NodeMatrixT;
  typedef Eigen::Matrix<Real, 1, nb_nodes> ShapeFunctionsT;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes> MappedGradientT;
  typedef Eigen::Matrix<Real, dimensionality, dimension> JacobianT;

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  /// @param mapped_coord The mapped coordinates
  /// @param shapeFunc Vector storing the result
  static void shape_function(const MappedCoordsT& mapped_coord, ShapeFunctionsT& shape_func);

  /// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
  /// mapped coordinates.
  /// @param mapped_coord The mapped coordinates where the gradient should be calculated
  /// @param result Storage for the resulting gradient matrix
  static void mapped_gradient(const MappedCoordsT& mapped_coord, MappedGradientT& result);

  /// Compute the Jacobian matrix
  /// In the case of the Triag3D element, this is the vector corresponding to the line segment
  /// @param mapped_coord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  static void jacobian(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result);

  /// Normal vector to the surface.
  /// @param mapped_coord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  static void normal(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, CoordsT& result);

  /// Volume of the cell. 0 in case of elements with a dimensionality that is less than
  /// the dimension of the problem
  static Real volume(const NodeMatrixT& nodes);

  /// The area of an element that represents a surface in the solution space, i.e.
  /// 1D elements in 2D space or 2D elements in 3D space
  static Real area(const NodeMatrixT& nodes);

  /// Given nodal values, write the interpolation
//   template<typename NodalValuesT, typename ValueT>
//   void operator()(const MappedCoo& mapped_coord, const NodalValuesT& nodal_values, ValueT& interpolation) const
//   {
//     cf_assert(mapped_coord.size() == dimensionality);
//     cf_assert(nodal_values.size() == nb_nodes);
// 
//     RealVector sf(nb_nodes);
//     shape_function(mapped_coord, sf);
// 
//     interpolation = sf[0]*nodal_values[0] + sf[1]*nodal_values[1] + sf[2]*nodal_values[2];
//   }

  virtual std::string element_type_name() const;

  /// The volume of an element with a dimensionality that is less than
  /// the dimension of the problem is 0.
  virtual Real compute_volume(const NodesT& coord) const;
	virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;
  virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
  virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

};

} // SF
} // Mesh
} // CF

#endif /* CF_Mesh_SF_Triag3DLagrangeP1 */
