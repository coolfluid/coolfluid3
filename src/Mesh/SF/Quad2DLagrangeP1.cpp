// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Line2DLagrangeP1.hpp"
#include "Quad2DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Quad2DLagrangeP1,ElementType,LibSF > aQuad2DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Quad2DLagrangeP1::Quad2DLagrangeP1(const std::string& name) : Quad2D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return volume(coord);
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP1::compute_centroid(const NodesT& coord , RealVector& centroid) const
{
  centroid[0] = 0.25*(coord(0,XX)+coord(1,XX)+coord(2,XX)+coord(3,XX));
  centroid[1] = 0.25*(coord(0,YY)+coord(1,YY)+coord(2,YY)+coord(3,YY));
}

////////////////////////////////////////////////////////////////////////////////

bool Quad2DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
  MappedCoordsT mapped_coord;
  mapped_coordinates(CoordsT(coord), nodes, mapped_coord);
  if( (mapped_coord[KSI] >= -1.0) &&
      (mapped_coord[ETA] >= -1.0) &&
      (mapped_coord[KSI] <=  1.0) &&
      (mapped_coord[ETA] <=  1.0))
  {
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP1::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(2)(4)(6);
    connectivity.face_node_counts.assign(4, 2);
    connectivity.face_nodes = boost::assign::list_of(0)(1)
                                                    (1)(2)
                                                    (2)(3)
                                                    (3)(0);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP1::face_connectivity() const
{
  return faces();
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Quad2DLagrangeP1::face_type(const CF::Uint face) const
{
  const static Line2DLagrangeP1 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP1::shape_function_value(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  SFQuadLagrangeP1::compute_value(mappedCoord,shapeFunc);
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP1::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord)
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

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP1::shape_function_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  SFQuadLagrangeP1::compute_gradient(mappedCoord,result);
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP1::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes)
{
  const Real x0 = nodes(0, XX);
  const Real y0 = nodes(0, YY);
  const Real x1 = nodes(1, XX);
  const Real y1 = nodes(1, YY);
  const Real x2 = nodes(2, XX);
  const Real y2 = nodes(2, YY);
  const Real x3 = nodes(3, XX);
  const Real y3 = nodes(3, YY);

  const Real xi  = mapped_coord[0];
  const Real eta = mapped_coord[1];
  return  ((x2 - x0)*(y3 - y1) + (x1 - x3)*(y2 - y0)
         -((x3 - x0)*(y2 - y1) + (x2 - x1)*(y0 - y3)) * eta
         -((x1 - x0)*(y3 - y2) + (x3 - x2)*(y0 - y1)) * xi)*0.125;

}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP1::jacobian_determinant(const RealVector& mapped_coord, const RealMatrix& nodes) const
{
  const Real x0 = nodes(0, XX);
  const Real y0 = nodes(0, YY);
  const Real x1 = nodes(1, XX);
  const Real y1 = nodes(1, YY);
  const Real x2 = nodes(2, XX);
  const Real y2 = nodes(2, YY);
  const Real x3 = nodes(3, XX);
  const Real y3 = nodes(3, YY);

  const Real xi  = mapped_coord[0];
  const Real eta = mapped_coord[1];
  return  ((x2 - x0)*(y3 - y1) + (x1 - x3)*(y2 - y0)
         -((x3 - x0)*(y2 - y1) + (x2 - x1)*(y0 - y3)) * eta
         -((x1 - x0)*(y3 - y2) + (x3 - x2)*(y0 - y1)) * xi)*0.125;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP1::jacobian(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result)
{
  JacobianCoefficients jc(nodes);

  const Real xi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  result(KSI,XX) = jc.bx + jc.dx*eta;
  result(KSI,YY) = jc.by + jc.dy*eta;
  result(ETA,XX) = jc.cx + jc.dx*xi;
  result(ETA,YY) = jc.cy + jc.dy*xi;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix Quad2DLagrangeP1::jacobian(const RealVector& mapped_coord, const RealMatrix& nodes) const
{
  RealMatrix result(dimensionality,dimension);

  JacobianCoefficients jc(nodes);

  const Real xi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  result(KSI,XX) = jc.bx + jc.dx*eta;
  result(KSI,YY) = jc.by + jc.dy*eta;
  result(ETA,XX) = jc.cx + jc.dx*xi;
  result(ETA,YY) = jc.cy + jc.dy*xi;

  return result;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP1::jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result)
{
  JacobianCoefficients jc(nodes);

  const Real xi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  result(KSI,XX) = jc.cy + jc.dy*xi;
  result(KSI,YY) = -jc.by - jc.dy*eta;
  result(ETA,XX) = -jc.cx - jc.dx*xi;
  result(ETA,YY) = jc.bx + jc.dx*eta;
}


RealVector Quad2DLagrangeP1::plane_jacobian_normal(const RealVector& mapped_coords,
                                                   const RealMatrix& nodes,
                                                   const CoordRef orientation) const
{
  const Real x0 = nodes(0,XX);
  const Real y0 = nodes(0,YY);

  const Real x1 = nodes(1,XX);
  const Real y1 = nodes(1,YY);

  const Real x2 = nodes(2,XX);
  const Real y2 = nodes(2,YY);

  const Real x3 = nodes(3,XX);
  const Real y3 = nodes(3,YY);

  const Real xi =  mapped_coords[KSI];
  const Real eta = mapped_coords[ETA];

  RealVector result(dimensionality);

  switch (orientation)
  {
    case KSI:
    {
      const Real dN0deta = -(1. - xi);
      const Real dN1deta = -(1. + xi);
      const Real dN2deta =  (1. + xi);
      const Real dN3deta =  (1. - xi);
      result[XX] = +0.25 * (y0*dN0deta + y1*dN1deta + y2*dN2deta + y3*dN3deta);
      result[YY] = -0.25 * (x0*dN0deta + x1*dN1deta + x2*dN2deta + x3*dN3deta);
      return result;
    }
    case ETA:
    {
      const Real dN0dxi = -(1. - eta);
      const Real dN1dxi =  (1. - eta);
      const Real dN2dxi =  (1. + eta);
      const Real dN3dxi = -(1. + eta);
      result[XX] = -0.25 * (y0*dN0dxi  + y1*dN1dxi  + y2*dN2dxi  + y3*dN3dxi);
      result[YY] = +0.25 * (x0*dN0dxi  + x1*dN1dxi  + x2*dN2dxi  + x3*dN3dxi);
      return result;
    }
    case ZTA:
      throw Common::ShouldNotBeHere(FromHere(),"");
  }
  throw Common::ShouldNotBeHere(FromHere(),"orientation not defined");
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP1::volume(const NodeMatrixT& nodes)
{
  const Real diagonalsProd =
    (nodes(2, XX) - nodes(0, XX)) * (nodes(3, YY) - nodes(1, YY)) -
    (nodes(2, YY) - nodes(0, YY)) * (nodes(3, XX) - nodes(1, XX));

  return 0.5*diagonalsProd;
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
