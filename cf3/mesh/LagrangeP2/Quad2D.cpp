// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "math/Consts.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP1/Quad2D.hpp"

#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP2/Quad2D.hpp"
#include "mesh/LagrangeP2/Line2D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

Eigen::Matrix<Real,Quad2D::nb_nodes,1> Quad2D::m_shapeFunc;
Eigen::Matrix<Real,Quad2D::nb_nodes,Quad2D::dimensionality> Quad2D::m_shapeFuncDerivs;

common::ComponentBuilder < ElementTypeT<Quad2D>, ElementType , LibLagrangeP2 >
   Quad2D_Builder(LibLagrangeP2::library_namespace()+"."+Quad2D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Quad2D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0)(3)(6)(9);
    connectivity.stride.assign(nb_faces, 3);
    connectivity.nodes = boost::assign::list_of(0)(1)(4)
                                               (1)(2)(5)
                                               (2)(3)(6)
                                               (3)(0)(7);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Quad2D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<LagrangeP2::Line2D> >(LagrangeP2::Line2D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid = nodes.row(8);
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2D::volume(const NodesT& nodes)
{
  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);
  const Real x3 = nodes(3, XX);
  const Real x4 = nodes(4, XX);
  const Real x5 = nodes(5, XX);
  const Real x6 = nodes(6, XX);
  const Real x7 = nodes(7, XX);
  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);
  const Real y3 = nodes(3, YY);
  const Real y4 = nodes(4, YY);
  const Real y5 = nodes(5, YY);
  const Real y6 = nodes(6, YY);
  const Real y7 = nodes(7, YY);

  return (4*((x7 - x4)*y0 + (x4 - x5)*y1 + (x5 - x6)*y2 + (x6 - x7)*y3) +
          x1*(y0 - y2 - 4*y4 + 4*y5) + x2*(y1 - y3 - 4*y5 + 4*y6) +
          x0*(y3 - y1 + 4*y4 - 4*y7) + x3*(y2 - y0 - 4*y6 + 4*y7)
         )/6.;

}

////////////////////////////////////////////////////////////////////////////////

Real Quad2D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2D::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  return jacobian(mapped_coord,nodes).determinant();
}

////////////////////////////////////////////////////////////////////////////////

void Quad2D::compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  // get mapped coordinates

  const Real ksi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  const Real ksi2 = ksi*ksi;
  const Real eta2 = eta*eta;
  const Real ksi_eta = ksi*eta;
  const Real ksi2_eta = ksi2*eta;
  const Real ksi_eta2 = ksi*eta2;

  // set shape function derivatives
  m_shapeFuncDerivs(0,KSI) =  0.25 * (eta - 2.*ksi_eta - eta2 + 2.*ksi_eta2);
  m_shapeFuncDerivs(1,KSI) = -0.25 * (eta + 2.*ksi_eta - eta2 - 2.*ksi_eta2);
  m_shapeFuncDerivs(2,KSI) =  0.25 * (eta + 2.*ksi_eta + eta2 + 2.*ksi_eta2);
  m_shapeFuncDerivs(3,KSI) = -0.25 * (eta - 2.*ksi_eta + eta2 - 2.*ksi_eta2);
  m_shapeFuncDerivs(4,KSI) = -0.5  * (-2.*ksi_eta + 2.*ksi_eta2);
  m_shapeFuncDerivs(5,KSI) =  0.5  * (1. - eta2 + 2.*ksi - 2.*ksi_eta2);
  m_shapeFuncDerivs(6,KSI) =  0.5  * (-2.*ksi_eta - 2.*ksi_eta2);
  m_shapeFuncDerivs(7,KSI) = -0.5  * (1. - eta2 - 2.*ksi + 2.*ksi_eta2);
  m_shapeFuncDerivs(8,KSI) =  2.*ksi_eta2 - 2.*ksi;

  m_shapeFuncDerivs(0,ETA) =  0.25 * (ksi - ksi2 - 2.*ksi_eta + 2.*ksi2_eta);
  m_shapeFuncDerivs(1,ETA) = -0.25 * (ksi + ksi2 - 2.*ksi_eta - 2.*ksi2_eta);
  m_shapeFuncDerivs(2,ETA) =  0.25 * (ksi + ksi2 + 2.*ksi_eta + 2.*ksi2_eta);
  m_shapeFuncDerivs(3,ETA) = -0.25 * (ksi - ksi2 + 2.*ksi_eta - 2.*ksi2_eta);
  m_shapeFuncDerivs(4,ETA) = -0.5 * (1. - ksi2 - 2.*eta + 2.*ksi2_eta);
  m_shapeFuncDerivs(5,ETA) =  0.5 * (-2.*ksi_eta - 2.*ksi2_eta);
  m_shapeFuncDerivs(6,ETA) =  0.5 * (1. - ksi2 + 2.*eta - 2.*ksi2_eta);
  m_shapeFuncDerivs(7,ETA) = -0.5 * (-2.*ksi_eta + 2.*ksi2_eta);
  m_shapeFuncDerivs(8,ETA) =  2.*ksi2_eta - 2.*eta;

  // evaluate Jacobian
  result(KSI,XX) = m_shapeFuncDerivs(0,KSI)*nodes(0,XX);
  result(ETA,XX) = m_shapeFuncDerivs(0,ETA)*nodes(0,XX);

  result(KSI,YY) = m_shapeFuncDerivs(0,KSI)*nodes(0,YY);
  result(ETA,YY) = m_shapeFuncDerivs(0,ETA)*nodes(0,YY);
  for (Uint n = 1; n < 9; ++n)
  {
    result(KSI,XX) += m_shapeFuncDerivs(n,KSI)*nodes(n,XX);
    result(ETA,XX) += m_shapeFuncDerivs(n,ETA)*nodes(n,XX);

    result(KSI,YY) += m_shapeFuncDerivs(n,KSI)*nodes(n,YY);
    result(ETA,YY) += m_shapeFuncDerivs(n,ETA)*nodes(n,YY);
  }
}

////////////////////////////////////////////////////////////////////////////////

Quad2D::JacobianT Quad2D::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
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
  // get mapped coordinates
  const Real ksi =  mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];
  const Real ksi2 = ksi*ksi;
  const Real eta2 = eta*eta;
  const Real ksi_eta = ksi*eta;

  if (orientation == 0)
  {
    const Real ksi2_eta = ksi2*eta;

    /// @note below, the derivatives of shapefunctions are computed, not the shapefunctions themselves
    m_shapeFunc[0] =  (ksi - ksi2 - 2.*(ksi_eta - ksi2_eta));
    m_shapeFunc[1] = -(ksi + ksi2 - 2.*(ksi_eta + ksi2_eta));
    m_shapeFunc[2] =  (ksi + ksi2 + 2.*(ksi_eta + ksi2_eta));
    m_shapeFunc[3] = -(ksi - ksi2 + 2.*(ksi_eta - ksi2_eta));
    m_shapeFunc[4] = -2. * (1. - ksi2 - 2.*(eta - ksi2_eta));
    m_shapeFunc[5] =  4. * (-ksi_eta - ksi2_eta);
    m_shapeFunc[6] =  2. * (1. - ksi2 + 2.*(eta - ksi2_eta));
    m_shapeFunc[7] = -4. * (-ksi_eta + ksi2_eta);
    m_shapeFunc[8] =  8. * (ksi2_eta - eta);

    result[XX] = +nodes(0,YY)*m_shapeFunc[0];
    result[YY] = -nodes(0,XX)*m_shapeFunc[0];
    for (Uint n = 1; n < 9; ++n)
    {
      result[XX] += nodes(n,YY)*m_shapeFunc[n];
      result[YY] -= nodes(n,XX)*m_shapeFunc[n];
    }
  }
  else
  {
    const Real ksi_eta2 = ksi*eta2;

    /// @note below, the derivatives of shapefunctions are computed, not the shapefunctions themselves
    m_shapeFunc[0] =  (eta - eta2 - 2.*(ksi_eta - ksi_eta2));
    m_shapeFunc[1] = -(eta - eta2 + 2.*(ksi_eta - ksi_eta2));
    m_shapeFunc[2] =  (eta + eta2 + 2.*(ksi_eta + ksi_eta2));
    m_shapeFunc[3] = -(eta + eta2 - 2.*(ksi_eta + ksi_eta2));
    m_shapeFunc[4] = -4. * (-ksi_eta + ksi_eta2);
    m_shapeFunc[5] =  2. * (1. - eta2 + 2.*(ksi - ksi_eta2));
    m_shapeFunc[6] =  4. * (-ksi_eta - ksi_eta2);
    m_shapeFunc[7] = -2. * (1. - eta2 - 2.*(ksi - ksi_eta2));
    m_shapeFunc[8] =  8. * (ksi_eta2 - ksi);

    result[XX] = -nodes(0,YY)*m_shapeFunc[0];
    result[YY] = +nodes(0,XX)*m_shapeFunc[0];
    for (Uint n = 1; n < 9; ++n)
    {
      result[XX] -= nodes(n,YY)*m_shapeFunc[n];
      result[YY] += nodes(n,XX)*m_shapeFunc[n];
    }
  }
  result *= 0.25;
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

  if (scp(nodes.row(0),nodes.row(7),coord,m_scale) * scp(nodes.row(0),coord,nodes.row(4),m_scale) < -tolerance)
      return false;
  if (scp(nodes.row(4),nodes.row(0),coord,m_scale) * scp(nodes.row(4),coord,nodes.row(1),m_scale) < -tolerance)
      return false;
  if (scp(nodes.row(1),nodes.row(4),coord,m_scale) * scp(nodes.row(1),coord,nodes.row(5),m_scale) < -tolerance)
      return false;
  if (scp(nodes.row(5),nodes.row(1),coord,m_scale) * scp(nodes.row(5),coord,nodes.row(2),m_scale) < -tolerance)
      return false;
  if (scp(nodes.row(2),nodes.row(5),coord,m_scale) * scp(nodes.row(2),coord,nodes.row(6),m_scale) < -tolerance)
      return false;
  if (scp(nodes.row(6),nodes.row(2),coord,m_scale) * scp(nodes.row(6),coord,nodes.row(3),m_scale) < -tolerance)
      return false;
  if (scp(nodes.row(3),nodes.row(6),coord,m_scale) * scp(nodes.row(3),coord,nodes.row(7),m_scale) < -tolerance)
      return false;
  if (scp(nodes.row(7),nodes.row(3),coord,m_scale) * scp(nodes.row(7),coord,nodes.row(0),m_scale) < -tolerance)
      return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2D::compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord)
{
  //  throw common::NotImplemented(FromHere(),"Implemented but untested");
  LagrangeP1::Quad2D::NodesT nodes_p1;
  nodes_p1 << nodes(0,XX) , nodes(0,YY), nodes(1,XX) , nodes(1,YY), nodes(2,XX) , nodes(2,YY), nodes(3,XX) , nodes(3,YY);
  LagrangeP1::Quad2D::compute_mapped_coordinate(coord,nodes_p1, mapped_coord);
  return;

  // Axes of the local coordinate system, centered around the centroid and going through the center of each face
//  SF::ValueT sf;
//  SF::compute_value(CoordsT(1.,0.), sf);
//  CoordsT ux = (sf*nodes).transpose();
//  SF::compute_value(CoordsT(0.,1.), sf);
//  CoordsT uy = (sf*nodes).transpose();

//  SF::compute_value(CoordsT(-1.,0.), sf);
//  CoordsT ux_neg = (sf*nodes).transpose();
//  SF::compute_value(CoordsT(0.,-1.), sf);
//  CoordsT uy_neg = (sf*nodes).transpose();

//  CoordsT centroid;
//  centroid[XX] = (ux[XX] + ux_neg[XX]) * 0.5;
//  centroid[YY] = (uy[YY] + uy_neg[YY]) * 0.5;

//  ux -= ux_neg;
//  uy -= uy_neg;

//  ux *= 0.5; // because the origin is at the center
//  uy *= 0.5;


//  const Real ux_len_inv = 1. / ux.norm();
//  const Real uy_len_inv = 1. / uy.norm();

//  ux *= ux_len_inv;
//  uy *= uy_len_inv;

//  std::cout << " centroid = " << centroid.transpose() << std::endl;
//  std::cout << "ux,uy = " << ux.transpose() << "   ,   " << uy.transpose() << std::endl;

//  // Normal vectors
//  CoordsT nx = CoordsT(ux[YY],-ux[XX]);
//  CoordsT ny = CoordsT(uy[YY],-uy[XX]);

//  // division factors for line-plane intersection
//  const Real fx = ux_len_inv / ux.dot(nx);
//  const Real fy = uy_len_inv / uy.dot(ny);

//  std::cout << "fx = " << fx << "   fy = " << fy << std::endl;
//  CoordsT diff = coord-centroid;

//  std::cout << "diff = " << diff.transpose() << std::endl;

//  CoordsT test;
//  const Real threshold = 1e-24; // 1e-12 squared, because we compare the squared distance
//  Uint nb_iters = 0;
//  // Initial guess will be correct if our element is a parallelepiped
//  mapped_coord[KSI] = diff.dot(nx) * fx;
//  mapped_coord[ETA] = diff.dot(ny) * fy;
//  while (nb_iters < 100 && diff.dot(diff) > threshold)
//  {
//    SF::compute_value(mapped_coord, sf);
//    test = (sf*nodes).transpose();
//    diff = coord - test;
//    test[XX] = diff.dot(nx) * fx;  // Transform difference to the relative coordinate system and
//    test[YY] = diff.dot(ny) * fy;  // use it to adjust our initial guess
//    mapped_coord += test;
//    ++nb_iters;
//  }

//  std::cout << "nb_iters = " << nb_iters << std::endl;

//  if(nb_iters > 100)
//    throw common::FailedToConverge(FromHere(), "Failed to find Hexa3DLagrangeP1 mapped coordinates");
}

////////////////////////////////////////////////////////////////////////////////

Quad2D::MappedCoordsT Quad2D::mapped_coordinate(const CoordsT& coord, const NodesT& nodes)
{
  MappedCoordsT result;
  compute_mapped_coordinate(coord,nodes,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

RealVector2 Quad2D::m_D;
Real Quad2D::m_scale;

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // mesh
} // cf3
