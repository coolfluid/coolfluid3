
// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Quad2DLagrangeP1_hpp
#define CF_Mesh_SF_Quad2DLagrangeP1_hpp

#include "Math/MatrixTypes.hpp"
#include "Mesh/Quad2D.hpp"

#include "Mesh/SF/LibSF.hpp"

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
struct MESH_SF_API Quad2DLagrangeP1  : public Quad2D {

  Quad2DLagrangeP1(const std::string& name = type_name());

  static std::string type_name() { return "Quad2DLagrangeP1"; }

/// Number of nodes
static const Uint nb_nodes = 4;

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
typedef Quad2D Support;
/// Compute the shape functions corresponding to the given
/// mapped coordinates
/// @param mappedCoord The mapped coordinates
/// @param shapeFunc Vector storing the result
static void shape_function(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  const Real xi  = mappedCoord[KSI];
  const Real eta = mappedCoord[ETA];

  shapeFunc[0] = 0.25 * (1.0 - xi) * (1.0 - eta);
  shapeFunc[1] = 0.25 * (1.0 + xi) * (1.0 - eta);
  shapeFunc[2] = 0.25 * (1.0 + xi) * (1.0 + eta);
  shapeFunc[3] = 0.25 * (1.0 - xi) * (1.0 + eta);
}

/// Compute Mapped Coordinates
/// @param coord contains the coordinates to be mapped
/// @param nodes contains the nodes
/// @param mappedCoord Store the output mapped coordinates
template<typename NodesT>
static void mapped_coordinates(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mappedCoord)
{
  const Real x = coord[XX];
  const Real y = coord[YY];
  
  const Real x0 = nodes(0, XX);
  const Real y0 = nodes(0, YY);
  const Real x1 = nodes(1, XX);
  const Real y1 = nodes(1, YY);
  const Real x2 = nodes(2, XX);
  const Real y2 = nodes(2, YY);
  const Real x3 = nodes(3, XX);
  const Real y3 = nodes(3, YY);
  
  JacobianCoefficients jc(nodes);
  if(jc.bx*jc.dy != jc.by*jc.dx) // non-zero quadratic term
  {
    mappedCoord[KSI] = (x*(y0 + y2 - y1 - y3) + x0*y3 + x2*y1 + y*(x1 + x3 - x0 - x2) - x1*y2 - x3*y0 + sqrt(-4*x*x0*y1*y3 - 4*x*x1*y0*y2 - 4*x*x2*y1*y3 - 4*x*x3*y0*y2 - 4*x0*x2*y*y1 - 4*x0*x2*y*y3 - 4*x1*x3*y*y0 - 4*x1*x3*y*y2 - 2*x*x0*y*y0 - 2*x*x0*y*y2 - 2*x*x1*y*y1 - 2*x*x1*y*y3 - 2*x*x2*y*y0 - 2*x*x2*y*y2 - 2*x*x3*y*y1 - 2*x*x3*y*y3 - 2*x0*x1*y2*y3 - 2*x0*x2*y0*y2 - 2*x0*x3*y1*y2 - 2*x1*x2*y0*y3 - 2*x1*x3*y1*y3 - 2*x2*x3*y0*y1 + 2*x*x0*y*y1 + 2*x*x0*y*y3 + 2*x*x0*y0*y2 + 2*x*x0*y1*y2 + 2*x*x0*y2*y3 + 2*x*x1*y*y0 + 2*x*x1*y*y2 + 2*x*x1*y0*y3 + 2*x*x1*y1*y3 + 2*x*x1*y2*y3 + 2*x*x2*y*y1 + 2*x*x2*y*y3 + 2*x*x2*y0*y1 + 2*x*x2*y0*y2 + 2*x*x2*y0*y3 + 2*x*x3*y*y0 + 2*x*x3*y*y2 + 2*x*x3*y0*y1 + 2*x*x3*y1*y2 + 2*x*x3*y1*y3 + 2*x0*x1*y*y2 + 2*x0*x1*y*y3 + 2*x0*x2*y*y0 + 2*x0*x2*y*y2 + 2*x0*x3*y*y1 + 2*x0*x3*y*y2 + 2*x1*x2*y*y0 + 2*x1*x2*y*y3 + 2*x1*x3*y*y1 + 2*x1*x3*y*y3 + 2*x2*x3*y*y0 + 2*x2*x3*y*y1 + 4*x0*x2*y1*y3 + 4*x1*x3*y0*y2 + x*x*y0*y0 + x*x*y1*y1 + x*x*y2*y2 + x*x*y3*y3 + x0*x0*y*y + x0*x0*y2*y2 + x1*x1*y*y + x1*x1*y3*y3 + x2*x2*y*y + x2*x2*y0*y0 + x3*x3*y*y + x3*x3*y1*y1 - 2*x*x0*y2*y2 - 2*x*x1*y3*y3 - 2*x*x2*y0*y0 - 2*x*x3*y1*y1 - 2*x0*x1*y*y - 2*x0*x3*y*y - 2*x1*x2*y*y - 2*x2*x3*y*y - 2*y*y0*x2*x2 - 2*y*y1*x3*x3 - 2*y*y2*x0*x0 - 2*y*y3*x1*x1 - 2*y0*y1*x*x - 2*y0*y3*x*x - 2*y1*y2*x*x - 2*y2*y3*x*x + 2*x0*x2*y*y + 2*x1*x3*y*y + 2*y0*y2*x*x + 2*y1*y3*x*x))/(x0*y3 + x1*y2 + x2*y0 + x3*y1 - x0*y2 - x1*y3 - x2*y1 - x3*y0);
  }
  else // linear equation
  {
    mappedCoord[KSI] = (x2*y0 + x2*y1 + x3*y0 + x3*y1 - x0*y2 - x0*y3 - x1*y2 - x1*y3 - 2*x*y0 - 2*x*y1 - 2*x2*y - 2*x3*y + 2*x*y2 + 2*x*y3 + 2*x0*y + 2*x1*y)/(-2*x*y0 - 2*x*y2 - 2*x0*y3 - 2*x1*y - 2*x2*y1 - 2*x3*y + 2*x*y1 + 2*x*y3 + 2*x0*y + 2*x1*y2 + 2*x2*y + 2*x3*y0);
  }
  mappedCoord[ETA] = -jc.cx != jc.dx*mappedCoord[KSI] ? (x - jc.ax - jc.bx * mappedCoord[KSI]) / (jc.cx + jc.dx*mappedCoord[KSI]) : (y - jc.ay - jc.by * mappedCoord[KSI]) / (jc.cy + jc.dy*mappedCoord[KSI]);
}

/// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
/// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
/// coordinates.
/// @param mappedCoord The mapped coordinates where the gradient should be calculated
/// @param result Storage for the resulting gradient matrix
static void mapped_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  const Real ksi  = mappedCoord[0];
  const Real eta = mappedCoord[1];

  result(XX, 0) = 0.25 * (-1 + eta);
  result(YY, 0) = 0.25 * (-1 + ksi);
  result(XX, 1) = 0.25 * ( 1 - eta);
  result(YY, 1) = 0.25 * (-1 - ksi);
  result(XX, 2) = 0.25 * ( 1 + eta);
  result(YY, 2) = 0.25 * ( 1 + ksi);
  result(XX, 3) = 0.25 * (-1 - eta);
  result(YY, 3) = 0.25 * ( 1 - ksi);
}

/// Compute the jacobian determinant at the given mapped coordinates
template<typename NodesT>
static Real jacobian_determinant(const MappedCoordsT& mappedCoord, const NodesT& nodes)
{
  const Real x0 = nodes(0, XX);
  const Real y0 = nodes(0, YY);
  const Real x1 = nodes(1, XX);
  const Real y1 = nodes(1, YY);
  const Real x2 = nodes(2, XX);
  const Real y2 = nodes(2, YY);
  const Real x3 = nodes(3, XX);
  const Real y3 = nodes(3, YY);
  
  const Real xi  = mappedCoord[0];
  const Real eta = mappedCoord[1];
  return  ((x2 - x0)*(y3 - y1) + (x1 - x3)*(y2 - y0)
         -((x3 - x0)*(y2 - y1) + (x2 - x1)*(y0 - y3)) * eta
         -((x1 - x0)*(y3 - y2) + (x3 - x2)*(y0 - y1)) * xi)*0.125;
         
}

/// Compute the Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
template<typename NodesT>
static void jacobian(const MappedCoordsT& mappedCoord, const NodesT& nodes, JacobianT& result)
{
  JacobianCoefficients jc(nodes);

  const Real xi = mappedCoord[KSI];
  const Real eta = mappedCoord[ETA];

  result(KSI,XX) = jc.bx + jc.dx*eta;
  result(KSI,YY) = jc.by + jc.dy*eta;
  result(ETA,XX) = jc.cx + jc.dx*xi;
  result(ETA,YY) = jc.cy + jc.dy*xi;
}

/// Compute the adjoint of Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting adjoint
template<typename NodesT>
static void jacobian_adjoint(const MappedCoordsT& mappedCoord, const NodesT& nodes, JacobianT& result)
{
  JacobianCoefficients jc(nodes);

  const Real xi = mappedCoord[KSI];
  const Real eta = mappedCoord[ETA];

  result(KSI,XX) = jc.cy + jc.dy*xi;
  result(KSI,YY) = -jc.by - jc.dy*eta;
  result(ETA,XX) = -jc.cx - jc.dx*xi;
  result(ETA,YY) = jc.bx + jc.dx*eta;
}

/// Volume of the cell
template<typename NodesT>
inline static Real volume(const NodesT& nodes)
{
  const Real diagonalsProd =
    (nodes(2, XX) - nodes(0, XX)) * (nodes(3, YY) - nodes(1, YY)) -
    (nodes(2, YY) - nodes(0, YY)) * (nodes(3, XX) - nodes(1, XX));

  return 0.5*diagonalsProd;
}

/// Connectivity info for the faces
static const FaceConnectivity& faces();

virtual std::string getElementTypeName() const;
virtual Real computeVolume(const NodesT& coord) const;
virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;
virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

private:
/// Convenience struct to easily access the elements that make up the jacobian
struct JacobianCoefficients
{
  Real ax, bx, cx, dx;
  Real ay, by, cy, dy;
  template<typename NodesT>
  JacobianCoefficients(const NodesT& nodes)
  {
    const Real x0 = nodes(0, XX);
    const Real y0 = nodes(0, YY);
    const Real x1 = nodes(1, XX);
    const Real y1 = nodes(1, YY);
    const Real x2 = nodes(2, XX);
    const Real y2 = nodes(2, YY);
    const Real x3 = nodes(3, XX);
    const Real y3 = nodes(3, YY);

    ax = 0.25*( x0 + x1 + x2 + x3);
    bx = 0.25*(-x0 + x1 + x2 - x3);
    cx = 0.25*(-x0 - x1 + x2 + x3);
    dx = 0.25*( x0 - x1 + x2 - x3);
    ay = 0.25*( y0 + y1 + y2 + y3);
    by = 0.25*(-y0 + y1 + y2 - y3);
    cy = 0.25*(-y0 - y1 + y2 + y3);
    dy = 0.25*( y0 - y1 + y2 - y3);
  }
};

};

} // SF
} // Mesh
} // CF

#endif /* CF_Mesh_SF_Quad2DLagrangeP1 */
