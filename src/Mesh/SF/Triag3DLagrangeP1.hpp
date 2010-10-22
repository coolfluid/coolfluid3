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
struct SF_API Triag3DLagrangeP1  : public Triag3D
{
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

  
  Triag3DLagrangeP1();

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  /// @param mapped_coord The mapped coordinates
  /// @param shapeFunc Vector storing the result
  static void shape_function(const MappedCoordsT& mapped_coord, ShapeFunctionsT& shape_func)
  {
    Triag2DLagrangeP1::shape_function(mapped_coord, shape_func);
  }

  /// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
  /// mapped coordinates.
  /// @param mapped_coord The mapped coordinates where the gradient should be calculated
  /// @param result Storage for the resulting gradient matrix
  static void mapped_gradient(const MappedCoordsT& mapped_coord, MappedGradientT& result)
  {
    Triag2DLagrangeP1::mapped_gradient(mapped_coord, result);
  }

  /// Compute the Jacobian matrix
  /// In the case of the Triag3D element, this is the vector corresponding to the line segment
  /// @param mapped_coord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  template<typename NodesT>
  static void jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
  {
    const Real x0 = nodes(0, XX);
    const Real x1 = nodes(1, XX);
    const Real x2 = nodes(2, XX);

    const Real y0 = nodes(0, YY);
    const Real y1 = nodes(1, YY);
    const Real y2 = nodes(2, YY);

    const Real z0 = nodes(0, ZZ);
    const Real z1 = nodes(1, ZZ);
    const Real z2 = nodes(2, ZZ);

    result(KSI,XX) = x1 - x0;
    result(KSI,YY) = y1 - y0;
    result(KSI,ZZ) = z1 - z0;

    result(ETA,XX) = x2 - x0;
    result(ETA,YY) = y2 - y0;
    result(ETA,ZZ) = z2 - z0;
  }

  /// Normal vector to the surface.
  /// @param mapped_coord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  template<typename NodesT>
  static void normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, CoordsT& result)
  {
    JacobianT jac;
    jacobian(mapped_coord, nodes, jac);

    result[XX] = jac(KSI,YY)*jac(ETA,ZZ) - jac(KSI,ZZ)*jac(ETA,YY);
    result[YY] = jac(KSI,ZZ)*jac(ETA,XX) - jac(KSI,XX)*jac(ETA,ZZ);
    result[ZZ] = jac(KSI,XX)*jac(ETA,YY) - jac(KSI,YY)*jac(ETA,XX);
  }

  /// Volume of the cell. 0 in case of elements with a dimensionality that is less than
  /// the dimension of the problem
  template<typename NodesT>
  static Real volume(const NodesT& nodes)
  {
    return 0.;
  }

  /// The area of an element that represents a surface in the solution space, i.e.
  /// 1D elements in 2D space or 2D elements in 3D space
  template<typename NodesT>
  static Real area(const NodesT& nodes)
  {
    CoordsT n;
    normal(MappedCoordsT::Zero(), nodes, n);
    return 0.5*n.norm();
  }

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

  virtual std::string getElementTypeName() const;

  /// The volume of an element with a dimensionality that is less than
  /// the dimension of the problem is 0.
  virtual Real computeVolume(const NodesT& coord) const;
	virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;
  virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
  virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

};

} // namespace SF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_SF_Triag3DLagrangeP1 */
