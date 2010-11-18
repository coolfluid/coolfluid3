// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Triag2DLagrangeP1_hpp
#define CF_Mesh_SF_Triag2DLagrangeP1_hpp

#include "Math/MatrixTypes.hpp"
#include "Math/MathConsts.hpp"

#include "Mesh/Triag2D.hpp"

#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

	using namespace Math::MathConsts;
	
/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// triangular element.
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
struct MESH_SF_API Triag2DLagrangeP1  : public Triag2D {

  
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

/// typedef for the supporting geometry
typedef Triag2D Support;
/// Compute the shape functions corresponding to the given
/// mapped coordinates
/// @param mappedCoord The mapped coordinates
/// @param shapeFunc Vector storing the result
static void shape_function(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  shapeFunc[0] = 1.0 - mappedCoord[0] - mappedCoord[1];
  shapeFunc[1] = mappedCoord[0];
  shapeFunc[2] = mappedCoord[1];
}

/// Compute Mapped Coordinates
/// @param coord contains the coordinates to be mapped
/// @param nodes contains the nodes
/// @param mappedCoord Store the output mapped coordinates
template<typename NodesT>
static void mapped_coordinates(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mappedCoord)
{
  const Real invDet = 1. / jacobian_determinant(nodes);

  mappedCoord[KSI] = invDet * ((nodes(2, YY) - nodes(0, YY))*coord[XX] + (nodes(0, XX) - nodes(2, XX))*coord[YY] - nodes(0, XX)*nodes(2, YY) + nodes(2, XX)*nodes(0, YY));
  mappedCoord[ETA] = invDet * ((nodes(0, YY) - nodes(1, YY))*coord[XX] + (nodes(1, XX) - nodes(0, XX))*coord[YY] + nodes(0, XX)*nodes(1, YY) - nodes(1, XX)*nodes(0, YY));
}

/// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
/// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
/// coordinates.
/// @param mappedCoord The mapped coordinates where the gradient should be calculated
/// @param result Storage for the resulting gradient matrix
static void mapped_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  result(XX, 0) = -1.;
  result(YY, 0) = -1.;
  result(XX, 1) = 1.;
  result(YY, 1) = 0.;
  result(XX, 2) = 0.;
  result(YY, 2) = 1.;
}

/// Compute the jacobian determinant at the given mapped coordinates
template<typename NodesT>
static Real jacobian_determinant(const MappedCoordsT& mappedCoord, const NodesT& nodes) {
  return jacobian_determinant(nodes);
}

/// Compute the Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
template<typename NodesT>
static void jacobian(const MappedCoordsT& mappedCoord, const NodesT& nodes, JacobianT& result)
{
  result(KSI,XX) = nodes(1, XX) - nodes(0, XX);
  result(KSI,YY) = nodes(1, YY) - nodes(0, YY);
  result(ETA,XX) = nodes(2, XX) - nodes(0, XX);
  result(ETA,YY) = nodes(2, YY) - nodes(0, YY);
}

/// Compute the adjoint of Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting adjoint
template<typename NodesT>
static void jacobian_adjoint(const MappedCoordsT& mappedCoord, const NodesT& nodes, JacobianT& result)
{
  result(KSI,XX) = nodes(2, YY) - nodes(0, YY);
  result(KSI,YY) = nodes(0, YY) - nodes(1, YY);
  result(ETA,XX) = nodes(0, XX) - nodes(2, XX);
  result(ETA,YY) = nodes(1, XX) - nodes(0, XX);
}

/// Volume of the cell
template<typename NodesT>
static Real volume(const NodesT& nodes)
{
  return 0.5 * jacobian_determinant(nodes);
}
	
template<typename NodesT>
static bool in_element(const CoordsT& coord, const NodesT& nodes)
{
	MappedCoordsT mapped_coord;
	mapped_coordinates(coord, nodes, mapped_coord);
	if( (mapped_coord[KSI] >= -eps()) &&
		 (mapped_coord[ETA] >= -eps()) &&
		 (mapped_coord.sum() <= 1.))
	{
		return true;
	}
	else
	{
		return false;
	}
}


static const FaceConnectivity& faces();

Triag2DLagrangeP1();
virtual std::string getElementTypeName() const;
virtual Real computeVolume(const NodesT& coord) const;
virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;
virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

private:

/// Helper function for reuse in volume() and jacobian_determinant()
template<typename NodesT>
static Real jacobian_determinant(const NodesT& nodes) {
  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);
  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);
  return (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
}

};

} // namespace SF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_SF_Triag2DLagrangeP1 */
