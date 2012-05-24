// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "math/Consts.hpp"
#include "math/Functions.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Hexa3D.hpp"
#include "mesh/LagrangeP1/Quad3D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

Eigen::Matrix<Real,Hexa3D::nb_nodes,1> Hexa3D::m_shapeFunc;
Eigen::Matrix<Real,Hexa3D::nb_nodes,Hexa3D::dimensionality> Hexa3D::m_shapeFuncDerivs;
Eigen::Matrix<Real,Hexa3D::dimension,1> Hexa3D::m_vec1;
Eigen::Matrix<Real,Hexa3D::dimension,1> Hexa3D::m_vec2;

common::ComponentBuilder < ElementTypeT<Hexa3D>, ElementType , LibLagrangeP1 >
   Hexa3D_Builder(LibLagrangeP1::library_namespace()+"."+Hexa3D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Hexa3D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0)(4)(8)(12)(16)(20);
    connectivity.stride.assign(nb_faces, 4);
    connectivity.nodes = boost::assign::list_of
        (0)(3)(2)(1)
        (4)(5)(6)(7)
        (0)(1)(5)(4)
        (1)(2)(6)(5)
        (3)(7)(6)(2)
        (0)(4)(7)(3);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Hexa3D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<LagrangeP1::Quad3D> >(LagrangeP1::Quad3D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Hexa3D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = 0.125*(nodes(0,XX)+nodes(1,XX)+nodes(2,XX)+nodes(3,XX)+nodes(4,XX)+nodes(5,XX)+nodes(6,XX)+nodes(7,XX));
  centroid[YY] = 0.125*(nodes(0,YY)+nodes(1,YY)+nodes(2,YY)+nodes(3,YY)+nodes(4,YY)+nodes(5,YY)+nodes(6,YY)+nodes(7,YY));
  centroid[ZZ] = 0.125*(nodes(0,ZZ)+nodes(1,ZZ)+nodes(2,ZZ)+nodes(3,ZZ)+nodes(4,ZZ)+nodes(5,ZZ)+nodes(6,ZZ)+nodes(7,ZZ));
}

////////////////////////////////////////////////////////////////////////////////

bool Hexa3D::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  for (Uint iFace=0; iFace<nb_faces; ++iFace)
  {
    if (!(is_orientation_inside(coord, nodes, iFace)))
      return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void Hexa3D::compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord)
{
  // Axes of the local coordinate system, centered around the centroid and going through the center of each face
  SF::ValueT sf;
  SF::compute_value(CoordsT(1.,0.,0.), sf);
  CoordsT ux = (sf*nodes).transpose();
  SF::compute_value(CoordsT(0.,1.,0.), sf);
  CoordsT uy = (sf*nodes).transpose();
  SF::compute_value(CoordsT(0.,0.,1.), sf);
  CoordsT uz = (sf*nodes).transpose();

  SF::compute_value(CoordsT(-1.,0.,0.), sf);
  CoordsT ux_neg = (sf*nodes).transpose();
  SF::compute_value(CoordsT(0.,-1.,0.), sf);
  CoordsT uy_neg = (sf*nodes).transpose();
  SF::compute_value(CoordsT(0.,0.,-1.), sf);
  CoordsT uz_neg = (sf*nodes).transpose();

  CoordsT centroid;
  centroid[XX] = (ux[XX] + ux_neg[XX]) * 0.5;
  centroid[YY] = (uy[YY] + uy_neg[YY]) * 0.5;
  centroid[ZZ] = (uz[ZZ] + uz_neg[ZZ]) * 0.5;

  ux -= ux_neg;
  uy -= uy_neg;
  uz -= uz_neg;

  ux *= 0.5; // because the origin is at the center
  uy *= 0.5;
  uz *= 0.5;

  const Real ux_len_inv = 1. / ux.norm();
  const Real uy_len_inv = 1. / uy.norm();
  const Real uz_len_inv = 1. / uz.norm();

  ux *= ux_len_inv;
  uy *= uy_len_inv;
  uz *= uz_len_inv;

  // Normal vectors
  CoordsT nyz = uy.cross(uz);
  CoordsT nxz = ux.cross(uz);
  CoordsT nxy = ux.cross(uy);

  // division factors for line-plane intersection
  const Real fx = ux_len_inv / ux.dot(nyz);
  const Real fy = uy_len_inv / uy.dot(nxz);
  const Real fz = uz_len_inv / uz.dot(nxy);

  CoordsT diff = coord-centroid;
  CoordsT test;
  const Real threshold = 1e-24; // 1e-12 squared, because we compare the squared distance
  Uint nb_iters = 0;
  // Initial guess will be correct if our element is a parallelepiped
  mapped_coord[KSI] = diff.dot(nyz) * fx;
  mapped_coord[ETA] = diff.dot(nxz) * fy;
  mapped_coord[ZTA] = diff.dot(nxy) * fz;
  while (nb_iters < 100 && diff.dot(diff) > threshold)
  {
    SF::compute_value(mapped_coord, sf);
    test = (sf*nodes).transpose();
    diff = coord - test;
    test[XX] = diff.dot(nyz) * fx;  // Transform difference to the relative coordinate system and
    test[YY] = diff.dot(nxz) * fy;  // use it to adjust our initial guess
    test[ZZ] = diff.dot(nxy) * fz;
    mapped_coord += test;
    ++nb_iters;
  }

  if(nb_iters > 100)
    throw common::FailedToConverge(FromHere(), "Failed to find Hexa3DLagrangeP1 mapped coordinates");
}

////////////////////////////////////////////////////////////////////////////////

Hexa3D::MappedCoordsT Hexa3D::mapped_coordinate(const CoordsT& coord, const NodesT& nodes)
{
  MappedCoordsT result;
  compute_mapped_coordinate(coord,nodes,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Hexa3D::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes)
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

  const Real z0 = nodes(0, ZZ);
  const Real z1 = nodes(1, ZZ);
  const Real z2 = nodes(2, ZZ);
  const Real z3 = nodes(3, ZZ);
  const Real z4 = nodes(4, ZZ);
  const Real z5 = nodes(5, ZZ);
  const Real z6 = nodes(6, ZZ);
  const Real z7 = nodes(7, ZZ);

  const Real xi   = mapped_coord[KSI];
  const Real eta  = mapped_coord[ETA];
  const Real zeta = mapped_coord[ZTA];

  const Real etaxi = (eta*xi);
  const Real etazeta = (eta*zeta);
  const Real xizeta = (xi*zeta);

  const Real f1 = 0.125*(-1.0 + eta + xi - etaxi);
  const Real f2 = 0.125*(-1.0 + eta - xi + etaxi);
  const Real f3 = 0.125*(-1.0 - eta - xi - etaxi);
  const Real f4 = 0.125*(-1.0 - eta + xi + etaxi);
  const Real f5 = 0.125*(+1.0 - eta - xi + etaxi);
  const Real f6 = 0.125*(+1.0 - eta + xi - etaxi);
  const Real f7 = 0.125*(+1.0 + eta + xi + etaxi);
  const Real f8 = 0.125*(+1.0 + eta - xi - etaxi);

  const Real f9  = 0.125*(-1.0 - eta - zeta - etazeta);
  const Real f10 = 0.125*(+1.0 + eta - zeta - etazeta);
  const Real f11 = 0.125*(+1.0 - eta + zeta - etazeta);
  const Real f12 = 0.125*(-1.0 + eta + zeta - etazeta);
  const Real f13 = 0.125*(+1.0 - eta - zeta + etazeta);
  const Real f14 = 0.125*(-1.0 + eta - zeta + etazeta);
  const Real f15 = 0.125*(-1.0 - eta + zeta + etazeta);
  const Real f16 = 0.125*(+1.0 + eta + zeta + etazeta);

  const Real f17 = 0.125*(-1.0 - xi - zeta - xizeta);
  const Real f18 = 0.125*(+1.0 + xi - zeta - xizeta);
  const Real f19 = 0.125*(+1.0 - xi + zeta - xizeta);
  const Real f20 = 0.125*(-1.0 + xi + zeta - xizeta);
  const Real f21 = 0.125*(+1.0 - xi - zeta + xizeta);
  const Real f22 = 0.125*(-1.0 + xi - zeta + xizeta);
  const Real f23 = 0.125*(-1.0 - xi + zeta + xizeta);
  const Real f24 = 0.125*(+1.0 + xi + zeta + xizeta);

  return (f1*z0 + f2*z1 + f3*z2 + f4*z3 + f5*z4 + f6*z5 + f7*z6 + f8*z7)*
                    (-((y7*f9 + y2*f10 + y5*f11 + y0*f12 + y1*f13 + y4*f14 + y3*f15 + y6*f16)*
                    (x5*f17 + x2*f18 + x7*f19 + x0*f20 + x3*f21 + x4*f22 + x1*f23 + x6*f24)) +
                    (x7*f9 + x2*f10 + x5*f11 + x0*f12 + x1*f13 + x4*f14 + x3*f15 + x6*f16)*
                    (y5*f17 + y2*f18 + y7*f19 + y0*f20 + y3*f21 + y4*f22 + y1*f23 + y6*f24)) -
                    (f1*y0 + f2*y1 + f3*y2 + f4*y3 + f5*y4 + f6*y5 + f7*y6 + f8*y7)*
                    (-((z7*f9 + z2*f10 + z5*f11 + z0*f12 + z1*f13 + z4*f14 + z3*f15 + z6*f16)*
                    (x5*f17 + x2*f18 + x7*f19 + x0*f20 + x3*f21 + x4*f22 + x1*f23 + x6*f24)) +
                    (x7*f9 + x2*f10 + x5*f11 + x0*f12 + x1*f13 + x4*f14 + x3*f15 + x6*f16)*
                    (z5*f17 + z2*f18 + z7*f19 + z0*f20 + z3*f21 + z4*f22 + z1*f23 + z6*f24)) +
                    (x2*f3 + x7*f8 + x5*f6 + x0*f1 + x4*f5 + x1*f2 + x3*f4 + x6*f7)*
                    (-((z7*f9 + z2*f10 + z5*f11 + z0*f12 + z1*f13 + z4*f14 + z3*f15 + z6*f16)*
                    (y5*f17 + y2*f18 + y7*f19 + y0*f20 + y3*f21 + y4*f22 + y1*f23 + y6*f24)) +
                    (y7*f9 + y2*f10 + y5*f11 + y0*f12 + y1*f13 + y4*f14 + y3*f15 + y6*f16)*
                    (z5*f17 + z2*f18 + z7*f19 + z0*f20 + z3*f21 + z4*f22 + z1*f23 + z6*f24));
}

////////////////////////////////////////////////////////////////////////////////

void Hexa3D::compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  const Real xi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];
  const Real zeta = mapped_coord[ZTA];

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

  const Real z0 = nodes(0, ZZ);
  const Real z1 = nodes(1, ZZ);
  const Real z2 = nodes(2, ZZ);
  const Real z3 = nodes(3, ZZ);
  const Real z4 = nodes(4, ZZ);
  const Real z5 = nodes(5, ZZ);
  const Real z6 = nodes(6, ZZ);
  const Real z7 = nodes(7, ZZ);

  result(KSI,XX) = 0.125 * (x1 + x2 + x5 + x6 - x0 - x3 - x4 - x7 + eta*(x0 + x2 + x4 + x6 - x1 - x3 - x5 - x7) + zeta*(x0 + x3 + x5 + x6 - x1 - x2 - x4 - x7) + eta*zeta*(x1 + x3 + x4 + x6 - x0 - x2 - x5 - x7));
  result(KSI,YY) = 0.125 * (y1 + y2 + y5 + y6 - y0 - y3 - y4 - y7 + eta*(y0 + y2 + y4 + y6 - y1 - y3 - y5 - y7) + zeta*(y0 + y3 + y5 + y6 - y1 - y2 - y4 - y7) + eta*zeta*(y1 + y3 + y4 + y6 - y0 - y2 - y5 - y7));
  result(KSI,ZZ) = 0.125 * (z1 + z2 + z5 + z6 - z0 - z3 - z4 - z7 + eta*(z0 + z2 + z4 + z6 - z1 - z3 - z5 - z7) + zeta*(z0 + z3 + z5 + z6 - z1 - z2 - z4 - z7) + eta*zeta*(z1 + z3 + z4 + z6 - z0 - z2 - z5 - z7));
  result(ETA,XX) = 0.125 * (x2 + x3 + x6 + x7 - x0 - x1 - x4 - x5 + xi*(x0 + x2 + x4 + x6 - x1 - x3 - x5 - x7) + zeta*(x0 + x1 + x6 + x7 - x2 - x3 - x4 - x5) + xi*zeta*(x1 + x3 + x4 + x6 - x0 - x2 - x5 - x7));
  result(ETA,YY) = 0.125 * (y2 + y3 + y6 + y7 - y0 - y1 - y4 - y5 + xi*(y0 + y2 + y4 + y6 - y1 - y3 - y5 - y7) + zeta*(y0 + y1 + y6 + y7 - y2 - y3 - y4 - y5) + xi*zeta*(y1 + y3 + y4 + y6 - y0 - y2 - y5 - y7));
  result(ETA,ZZ) = 0.125 * (z2 + z3 + z6 + z7 - z0 - z1 - z4 - z5 + xi*(z0 + z2 + z4 + z6 - z1 - z3 - z5 - z7) + zeta*(z0 + z1 + z6 + z7 - z2 - z3 - z4 - z5) + xi*zeta*(z1 + z3 + z4 + z6 - z0 - z2 - z5 - z7));
  result(ZTA,XX) = 0.125 * (x4 + x5 + x6 + x7 - x0 - x1 - x2 - x3 + eta*(x0 + x1 + x6 + x7 - x2 - x3 - x4 - x5) + xi*(x0 + x3 + x5 + x6 - x1 - x2 - x4 - x7) + eta*xi*(x1 + x3 + x4 + x6 - x0 - x2 - x5 - x7));
  result(ZTA,YY) = 0.125 * (y4 + y5 + y6 + y7 - y0 - y1 - y2 - y3 + eta*(y0 + y1 + y6 + y7 - y2 - y3 - y4 - y5) + xi*(y0 + y3 + y5 + y6 - y1 - y2 - y4 - y7) + eta*xi*(y1 + y3 + y4 + y6 - y0 - y2 - y5 - y7));
  result(ZTA,ZZ) = 0.125 * (z4 + z5 + z6 + z7 - z0 - z1 - z2 - z3 + eta*(z0 + z1 + z6 + z7 - z2 - z3 - z4 - z5) + xi*(z0 + z3 + z5 + z6 - z1 - z2 - z4 - z7) + eta*xi*(z1 + z3 + z4 + z6 - z0 - z2 - z5 - z7));
}

////////////////////////////////////////////////////////////////////////////////

Hexa3D::JacobianT Hexa3D::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

void Hexa3D::compute_jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  JacobianT J = jacobian(mapped_coord,nodes);
  result(0, 0) =  (J(1, 1)*J(2, 2) - J(1, 2)*J(2, 1));
  result(0, 1) = -(J(0, 1)*J(2, 2) - J(0, 2)*J(2, 1));
  result(0, 2) =  (J(0, 1)*J(1, 2) - J(1, 1)*J(0, 2));
  result(1, 0) = -(J(1, 0)*J(2, 2) - J(1, 2)*J(2, 0));
  result(1, 1) =  (J(0, 0)*J(2, 2) - J(0, 2)*J(2, 0));
  result(1, 2) = -(J(0, 0)*J(1, 2) - J(0, 2)*J(1, 0));
  result(2, 0) =  (J(1, 0)*J(2, 1) - J(1, 1)*J(2, 0));
  result(2, 1) = -(J(0, 0)*J(2, 1) - J(0, 1)*J(2, 0));
  result(2, 2) =  (J(0, 0)*J(1, 1) - J(0, 1)*J(1, 0));
}

////////////////////////////////////////////////////////////////////////////////

Real Hexa3D::volume(const NodesT& nodes)
{
  return 8*jacobian_determinant(MappedCoordsT::Zero(), nodes);
}

////////////////////////////////////////////////////////////////////////////////

Real Hexa3D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

bool Hexa3D::is_orientation_inside(const CoordsT& coord, const NodesT& nodes, const Uint face)
{
  static const Real tolerance = 1e-11;

  //test according to http://graphics.ethz.ch/~peikert/personal/HexCellTest/

  const Uint a = faces().nodes_range(face)[3];
  const Uint b = faces().nodes_range(face)[2];
  const Uint c = faces().nodes_range(face)[1];
  const Uint d = faces().nodes_range(face)[0];

  RealMatrix3 M;
  M.col(0) = nodes.row(b) - nodes.row(a);
  M.col(1) = nodes.row(d) - nodes.row(a);
  M.col(2) = nodes.row(c) - nodes.row(a);
  const CoordsT pp = coord.transpose()  - nodes.row(a);

  const CoordsT bp_x_dp = M.col(0).cross(M.col(1));
  const Real h = bp_x_dp.dot(M.col(2));
  if (h != 0)
  {
    RealMatrix3 T;
    T << 1,  0,  1,
         0,  1,  1,
         0,  0,  h;

    // Do transformation
    RealVector3 ppp = T*M.inverse()*pp;

    if (ppp[ZZ] < h*ppp[XX]*ppp[YY])
      return false;
  }
  else
  {
    if (bp_x_dp.dot(pp) < 0)
      return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

Hexa3D::CoordsT Hexa3D::plane_jacobian_normal(const CoordsT& mapped_coords,
                                              const NodesT& nodes,
                                              const CoordRef orientation)
{
  CoordsT result;
  compute_plane_jacobian_normal(mapped_coords,nodes,orientation,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

void Hexa3D::compute_plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation, CoordsT& result)
{
  const Real xi =  mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];
  const Real zta = mapped_coord[ZTA];

  const Real a1 = 1. + xi;
  const Real a2 = 1. - xi;
  const Real b1 = 1. + eta;
  const Real b2 = 1. - eta;
  const Real c1 = 1. + zta;
  const Real c2 = 1. - zta;

  switch (orientation)
  {
    case KSI:

      m_shapeFuncDerivs(0,ETA) = -a2*c2;
      m_shapeFuncDerivs(1,ETA) = -a1*c2;
      m_shapeFuncDerivs(2,ETA) =  a1*c2;
      m_shapeFuncDerivs(3,ETA) =  a2*c2;
      m_shapeFuncDerivs(4,ETA) = -a2*c1;
      m_shapeFuncDerivs(5,ETA) = -a1*c1;
      m_shapeFuncDerivs(6,ETA) =  a1*c1;
      m_shapeFuncDerivs(7,ETA) =  a2*c1;

      m_shapeFuncDerivs(0,ZTA) = -a2*b2;
      m_shapeFuncDerivs(1,ZTA) = -a1*b2;
      m_shapeFuncDerivs(2,ZTA) = -a1*b1;
      m_shapeFuncDerivs(3,ZTA) = -a2*b1;
      m_shapeFuncDerivs(4,ZTA) =  b2*a2;
      m_shapeFuncDerivs(5,ZTA) =  b2*a1;
      m_shapeFuncDerivs(6,ZTA) =  b1*a1;
      m_shapeFuncDerivs(7,ZTA) =  b1*a2;

      m_vec1 = m_shapeFuncDerivs(0,ETA)*(nodes.row(0));
      m_vec2 = m_shapeFuncDerivs(0,ZTA)*(nodes.row(0));
      for (Uint in = 1; in < 8; ++in)
      {
        m_vec1 += m_shapeFuncDerivs(in,ETA)*(nodes.row(in));
        m_vec2 += m_shapeFuncDerivs(in,ZTA)*(nodes.row(in));
      }
      break;

    case ETA:

      m_shapeFuncDerivs(0,ZTA) = -a2*b2;
      m_shapeFuncDerivs(1,ZTA) = -a1*b2;
      m_shapeFuncDerivs(2,ZTA) = -a1*b1;
      m_shapeFuncDerivs(3,ZTA) = -a2*b1;
      m_shapeFuncDerivs(4,ZTA) =  b2*a2;
      m_shapeFuncDerivs(5,ZTA) =  b2*a1;
      m_shapeFuncDerivs(6,ZTA) =  b1*a1;
      m_shapeFuncDerivs(7,ZTA) =  b1*a2;

      m_shapeFuncDerivs(0,KSI) = -b2*c2;
      m_shapeFuncDerivs(1,KSI) =  b2*c2;
      m_shapeFuncDerivs(2,KSI) =  b1*c2;
      m_shapeFuncDerivs(3,KSI) = -b1*c2;
      m_shapeFuncDerivs(4,KSI) = -b2*c1;
      m_shapeFuncDerivs(5,KSI) =  b2*c1;
      m_shapeFuncDerivs(6,KSI) =  b1*c1;
      m_shapeFuncDerivs(7,KSI) = -b1*c1;

      m_vec1 = m_shapeFuncDerivs(0,ZTA)*(nodes.row(0));
      m_vec2 = m_shapeFuncDerivs(0,KSI)*(nodes.row(0));
      for (Uint in = 1; in < 8; ++in)
      {
        m_vec1 += m_shapeFuncDerivs(in,ZTA)*(nodes.row(in));
        m_vec2 += m_shapeFuncDerivs(in,KSI)*(nodes.row(in));
      }
      break;

    case ZTA:

      m_shapeFuncDerivs(0,KSI) = -b2*c2;
      m_shapeFuncDerivs(1,KSI) =  b2*c2;
      m_shapeFuncDerivs(2,KSI) =  b1*c2;
      m_shapeFuncDerivs(3,KSI) = -b1*c2;
      m_shapeFuncDerivs(4,KSI) = -b2*c1;
      m_shapeFuncDerivs(5,KSI) =  b2*c1;
      m_shapeFuncDerivs(6,KSI) =  b1*c1;
      m_shapeFuncDerivs(7,KSI) = -b1*c1;

      m_shapeFuncDerivs(0,ETA) = -a2*c2;
      m_shapeFuncDerivs(1,ETA) = -a1*c2;
      m_shapeFuncDerivs(2,ETA) =  a1*c2;
      m_shapeFuncDerivs(3,ETA) =  a2*c2;
      m_shapeFuncDerivs(4,ETA) = -a2*c1;
      m_shapeFuncDerivs(5,ETA) = -a1*c1;
      m_shapeFuncDerivs(6,ETA) =  a1*c1;
      m_shapeFuncDerivs(7,ETA) =  a2*c1;

      m_vec1 = m_shapeFuncDerivs(0,KSI)*(nodes.row(0));
      m_vec2 = m_shapeFuncDerivs(0,ETA)*(nodes.row(0));
      for (Uint in = 1; in < 8; ++in)
      {
        m_vec1 += m_shapeFuncDerivs(in,KSI)*(nodes.row(in));
        m_vec2 += m_shapeFuncDerivs(in,ETA)*(nodes.row(in));
      }
      break;

    default:
      throw common::ShouldNotBeHere(FromHere(),"Wrong orientation");
  }

  // compute normal
  math::Functions::cross_product(m_vec1,m_vec2,result);
  result *= 0.015625;
}
////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
