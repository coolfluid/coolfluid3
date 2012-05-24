// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "math/Consts.hpp"
#include "math/Functions.hpp"
#include "math/Checks.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Quad2D.hpp"
#include "mesh/LagrangeP1/Line2D.hpp"

using namespace cf3::math::Checks;
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
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<LagrangeP1::Line2D> >(LagrangeP1::Line2D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = 0.25*(nodes(0,XX)+nodes(1,XX)+nodes(2,XX)+nodes(3,XX));
  centroid[YY] = 0.25*(nodes(0,YY)+nodes(1,YY)+nodes(2,YY)+nodes(3,YY));
}

////////////////////////////////////////////////////////////////////////////////

// scalar_cross_product
Real scp(const Quad2D::CoordsT& P0, const Quad2D::CoordsT& P1, const Quad2D::CoordsT& P2, const Real& scale=1.)
{
  return (scale*P1[XX]-scale*P0[XX])*(scale*P2[YY]-scale*P0[YY])-(scale*P2[XX]-scale*P0[XX])*(scale*P1[YY]-scale*P0[YY]);
}

////////////////////////////////////////////////////////////////////////////////

bool Quad2D::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  // Description found in http://hal.archives-ouvertes.fr/docs/00/12/27/30/PDF/exact_interpolation.pdf

  static const Real tolerance = 1e-6;

  if (coord[XX] > nodes.col(XX).maxCoeff()+tolerance)
    return false;
  if (coord[XX] < nodes.col(XX).minCoeff()-tolerance)
    return false;
  if (coord[YY] > nodes.col(YY).maxCoeff()+tolerance)
    return false;
  if (coord[YY] < nodes.col(YY).minCoeff()-tolerance)
    return false;


  m_D <<
      nodes.col(XX).maxCoeff()-nodes.col(XX).minCoeff(),
      nodes.col(YY).maxCoeff()-nodes.col(YY).minCoeff();
  m_scale = 1./m_D.minCoeff();

  if (scp(nodes.row(0),nodes.row(Quad2D::nb_nodes-1),coord,m_scale) * scp(nodes.row(0),coord,nodes.row(1),m_scale) < -tolerance)
      return false;
  for (Uint i=1; i<Quad2D::nb_nodes-1; ++i)
  {
    if (scp(nodes.row(i),nodes.row(i-1),coord,m_scale) * scp(nodes.row(i),coord,nodes.row(i+1),m_scale) < -tolerance)
        return false;
  }
  if (scp(nodes.row(Quad2D::nb_nodes-1),nodes.row(Quad2D::nb_nodes-2),coord,m_scale) * scp(nodes.row(Quad2D::nb_nodes-1),coord,nodes.row(0),m_scale) < -tolerance)
      return false;

  return true;
}
////////////////////////////////////////////////////////////////////////////////

void Quad2D::compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord)
{

  // Description found in http://hal.archives-ouvertes.fr/docs/00/12/27/30/PDF/exact_interpolation.pdf

  m_D <<
      nodes.col(XX).maxCoeff()-nodes.col(XX).minCoeff(),
      nodes.col(YY).maxCoeff()-nodes.col(YY).minCoeff();
  m_scale = 1./m_D.minCoeff();

  const Real x = coord[XX] * m_scale;
  const Real y = coord[YY] * m_scale;

  const Real xn1 = nodes(0, XX)  * m_scale ;
  const Real yn1 = nodes(0, YY)  * m_scale ;
  const Real xn2 = nodes(1, XX)  * m_scale ;
  const Real yn2 = nodes(1, YY)  * m_scale ;
  const Real xn3 = nodes(2, XX)  * m_scale ;
  const Real yn3 = nodes(2, YY)  * m_scale ;
  const Real xn4 = nodes(3, XX)  * m_scale ;
  const Real yn4 = nodes(3, YY)  * m_scale ;

  const Real a0 = 0.25*( (xn1+xn2) + (xn3+xn4) );
  const Real a1 = 0.25*( (xn2-xn1) + (xn3-xn4) );
  const Real a2 = 0.25*( (xn3+xn4) - (xn1+xn2) );
  const Real a3 = 0.25*( (xn1-xn2) + (xn3-xn4) );

  const Real b0 = 0.25*( (yn1+yn2) + (yn3+yn4) );
  const Real b1 = 0.25*( (yn2-yn1) + (yn3-yn4) );
  const Real b2 = 0.25*( (yn3+yn4) - (yn1+yn2) );
  const Real b3 = 0.25*( (yn1-yn2) + (yn3-yn4) );


  // x = a0 + a1*xi + a2*eta + a3*xi*eta
  // y = b0 + b1*xi + b2*eta + b3*xi*eta

  // We have to invert these:
  //
  // xi = (y0*a3+a1*b2)/(a3*b1-a1*b3);
  // A*eta^2 + B*eta + C = 0
  //
  // with
  const Real x0 = x-a0;
  const Real y0 = y-b0;
  const Real A = a3*b2-a2*b3;
  const Real B = (x0*b3+a1*b2) - (y0*a3+a2*b1);
  const Real C = x0*b1-y0*a1;

  // 1) Solve for eta
  if (std::abs(A)<1e-10)
  {
    mapped_coord[ETA] = -C/B;
  }
  else
  {
    const Real B2 = B*B;
    Real q = -0.5*(B + math::Functions::sign(B)*std::sqrt(B2-4*A*C));
    Real eta1 = q/A;
    Real eta2 = C/q;
    mapped_coord[ETA] = (std::abs(eta1)<=std::abs(eta2)) ? eta1 : eta2;
  }

  // if  (a1+a3*eta == 0)  then we will divide by zero in to calculate xi.
  if ( std::abs(a1+a3*mapped_coord[ETA]) < 1e-8 )
  {
    Real tolerance=1e-8;

    if (std::abs(a3)>tolerance)
    {
      mapped_coord[ETA] = -a1/a3;
      mapped_coord[KSI] = (y0*a3+a1*b2)/(a3*b1-a1*b3);
    }
    else // if (std::abs(a3)<=tolerance)
    {
      mapped_coord[ETA] = x0/a2;
      mapped_coord[KSI] = (y0*a2-x0*b2)/(a2*b1+x0*b3);
    }
  }
  else // we won't divide by zero so we're good.
  {
    mapped_coord[KSI] = (x0-a2*mapped_coord[ETA])/(a1+a3*mapped_coord[ETA]);
  }

  // clip to make it more robust
  mapped_coord[KSI] = math::Functions::sign(mapped_coord[KSI])*std::min(1.,std::abs(mapped_coord[KSI]));
  mapped_coord[ETA] = math::Functions::sign(mapped_coord[ETA])*std::min(1.,std::abs(mapped_coord[ETA]));
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

RealVector2 Quad2D::m_D;
Real Quad2D::m_scale;

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
