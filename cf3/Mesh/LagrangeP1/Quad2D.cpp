// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/CBuilder.hpp"

#include "math/Consts.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Quad2D.hpp"
#include "mesh/LagrangeP1/Line2D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Quad2D>, ElementType , LibLagrangeP1 >
   Quad2D_Builder(LibLagrangeP1::library_namespace()+"."+Quad2D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Quad2D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0)(2)(4)(6);
    connectivity.stride.assign(nb_faces, 2);
    connectivity.nodes = boost::assign::list_of(0)(1)
                                               (1)(2)
                                               (2)(3)
                                               (3)(0);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Quad2D::face_type(const cf3::Uint face)
{
  static const ElementType::ConstPtr facetype( common::allocate_component<ElementTypeT<LagrangeP1::Line2D> >(LagrangeP1::Line2D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = 0.25*(nodes(0,XX)+nodes(1,XX)+nodes(2,XX)+nodes(3,XX));
  centroid[YY] = 0.25*(nodes(0,YY)+nodes(1,YY)+nodes(2,YY)+nodes(3,YY));
}

////////////////////////////////////////////////////////////////////////////////

bool Quad2D::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  MappedCoordsT mapped_coord = mapped_coordinate(coord,nodes);
  if( (mapped_coord[KSI] >= -1.0-math::Consts::eps()) &&
      (mapped_coord[ETA] >= -1.0-math::Consts::eps()) &&
      (mapped_coord[KSI] <=  1.0+math::Consts::eps()) &&
      (mapped_coord[ETA] <=  1.0+math::Consts::eps()))
  {
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

void Quad2D::compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord)
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

  // Threshold used to determine if the result is quadratic
  const Real quadratic_threshold = 1e-3;

  JacobianCoefficients jc(nodes);
  const Real nominator_ksi = (x0*y3 + x1*y2 + x2*y0 + x3*y1 - x0*y2 - x1*y3 - x2*y1 - x3*y0);
  if(fabs(nominator_ksi) > quadratic_threshold) // non-zero quadratic term
  {
    mapped_coord[KSI] = (x*(y0 + y2 - y1 - y3) + x0*y3 + x2*y1 + y*(x1 + x3 - x0 - x2) - x1*y2 - x3*y0 + sqrt(-4*x*x0*y1*y3 - 4*x*x1*y0*y2 - 4*x*x2*y1*y3 - 4*x*x3*y0*y2 - 4*x0*x2*y*y1 - 4*x0*x2*y*y3 - 4*x1*x3*y*y0 - 4*x1*x3*y*y2 - 2*x*x0*y*y0 - 2*x*x0*y*y2 - 2*x*x1*y*y1 - 2*x*x1*y*y3 - 2*x*x2*y*y0 - 2*x*x2*y*y2 - 2*x*x3*y*y1 - 2*x*x3*y*y3 - 2*x0*x1*y2*y3 - 2*x0*x2*y0*y2 - 2*x0*x3*y1*y2 - 2*x1*x2*y0*y3 - 2*x1*x3*y1*y3 - 2*x2*x3*y0*y1 + 2*x*x0*y*y1 + 2*x*x0*y*y3 + 2*x*x0*y0*y2 + 2*x*x0*y1*y2 + 2*x*x0*y2*y3 + 2*x*x1*y*y0 + 2*x*x1*y*y2 + 2*x*x1*y0*y3 + 2*x*x1*y1*y3 + 2*x*x1*y2*y3 + 2*x*x2*y*y1 + 2*x*x2*y*y3 + 2*x*x2*y0*y1 + 2*x*x2*y0*y2 + 2*x*x2*y0*y3 + 2*x*x3*y*y0 + 2*x*x3*y*y2 + 2*x*x3*y0*y1 + 2*x*x3*y1*y2 + 2*x*x3*y1*y3 + 2*x0*x1*y*y2 + 2*x0*x1*y*y3 + 2*x0*x2*y*y0 + 2*x0*x2*y*y2 + 2*x0*x3*y*y1 + 2*x0*x3*y*y2 + 2*x1*x2*y*y0 + 2*x1*x2*y*y3 + 2*x1*x3*y*y1 + 2*x1*x3*y*y3 + 2*x2*x3*y*y0 + 2*x2*x3*y*y1 + 4*x0*x2*y1*y3 + 4*x1*x3*y0*y2 + x*x*y0*y0 + x*x*y1*y1 + x*x*y2*y2 + x*x*y3*y3 + x0*x0*y*y + x0*x0*y2*y2 + x1*x1*y*y + x1*x1*y3*y3 + x2*x2*y*y + x2*x2*y0*y0 + x3*x3*y*y + x3*x3*y1*y1 - 2*x*x0*y2*y2 - 2*x*x1*y3*y3 - 2*x*x2*y0*y0 - 2*x*x3*y1*y1 - 2*x0*x1*y*y - 2*x0*x3*y*y - 2*x1*x2*y*y - 2*x2*x3*y*y - 2*y*y0*x2*x2 - 2*y*y1*x3*x3 - 2*y*y2*x0*x0 - 2*y*y3*x1*x1 - 2*y0*y1*x*x - 2*y0*y3*x*x - 2*y1*y2*x*x - 2*y2*y3*x*x + 2*x0*x2*y*y + 2*x1*x3*y*y + 2*y0*y2*x*x + 2*y1*y3*x*x))/nominator_ksi;
  }
  else // linear equation
  {
    cf3_assert(fabs(-2*x*y0 - 2*x*y2 - 2*x0*y3 - 2*x1*y - 2*x2*y1 - 2*x3*y + 2*x*y1 + 2*x*y3 + 2*x0*y + 2*x1*y2 + 2*x2*y + 2*x3*y0) > quadratic_threshold);
    mapped_coord[KSI] = (x2*y0 + x2*y1 + x3*y0 + x3*y1 - x0*y2 - x0*y3 - x1*y2 - x1*y3 - 2*x*y0 - 2*x*y1 - 2*x2*y - 2*x3*y + 2*x*y2 + 2*x*y3 + 2*x0*y + 2*x1*y)/(-2*x*y0 - 2*x*y2 - 2*x0*y3 - 2*x1*y - 2*x2*y1 - 2*x3*y + 2*x*y1 + 2*x*y3 + 2*x0*y + 2*x1*y2 + 2*x2*y + 2*x3*y0);
  }

  const Real nominator_eta = (jc.cx + jc.dx*mapped_coord[KSI]);
  if(fabs(nominator_eta > quadratic_threshold))
  {
    mapped_coord[ETA] = (x - jc.ax - jc.bx * mapped_coord[KSI]) / nominator_eta;
  }
  else
  {
    cf3_assert(fabs(jc.cy + jc.dy*mapped_coord[KSI]) > quadratic_threshold);
    mapped_coord[ETA] = (y - jc.ay - jc.by * mapped_coord[KSI]) / (jc.cy + jc.dy*mapped_coord[KSI]);
  }
}

////////////////////////////////////////////////////////////////////////////////

Quad2D::MappedCoordsT Quad2D::mapped_coordinate(const CoordsT& coord, const NodesT& nodes)
{
  MappedCoordsT result;
  compute_mapped_coordinate(coord,nodes,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2D::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes) {
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

void Quad2D::compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
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

Quad2D::JacobianT Quad2D::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2D::compute_jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  JacobianCoefficients jc(nodes);

  const Real xi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  result(KSI,XX) = jc.cy + jc.dy*xi;
  result(KSI,YY) = -jc.by - jc.dy*eta;
  result(ETA,XX) = -jc.cx - jc.dx*xi;
  result(ETA,YY) = jc.bx + jc.dx*eta;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2D::volume(const NodesT& nodes)
{
  const Real diagonalsProd =
    (nodes(2, XX) - nodes(0, XX)) * (nodes(3, YY) - nodes(1, YY)) -
    (nodes(2, YY) - nodes(0, YY)) * (nodes(3, XX) - nodes(1, XX));

  return 0.5*diagonalsProd;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Quad2D::CoordsT Quad2D::plane_jacobian_normal(const CoordsT& mapped_coords,
                                              const NodesT& nodes,
                                              const CoordRef orientation)
{
  CoordsT result;
  compute_plane_jacobian_normal(mapped_coords,nodes,orientation,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2D::compute_plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation, CoordsT& result)
{
  const Real x0 = nodes(0,XX);
  const Real y0 = nodes(0,YY);

  const Real x1 = nodes(1,XX);
  const Real y1 = nodes(1,YY);

  const Real x2 = nodes(2,XX);
  const Real y2 = nodes(2,YY);

  const Real x3 = nodes(3,XX);
  const Real y3 = nodes(3,YY);

  const Real xi =  mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

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
      return;
    }
    case ETA:
    {
      const Real dN0dxi = -(1. - eta);
      const Real dN1dxi =  (1. - eta);
      const Real dN2dxi =  (1. + eta);
      const Real dN3dxi = -(1. + eta);
      result[XX] = -0.25 * (y0*dN0dxi  + y1*dN1dxi  + y2*dN2dxi  + y3*dN3dxi);
      result[YY] = +0.25 * (x0*dN0dxi  + x1*dN1dxi  + x2*dN2dxi  + x3*dN3dxi);
      return;
    }
    case ZTA:
      throw common::ShouldNotBeHere(FromHere(),"");
  }
  throw common::ShouldNotBeHere(FromHere(),"orientation not defined");
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
