// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Hexa3DLagrangeP1.hpp"
#include "Quad3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Hexa3DLagrangeP1,
                           ElementType,
                           LibSF >
aHexa3DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Hexa3DLagrangeP1::Hexa3DLagrangeP1(const std::string& name) : Hexa3D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

std::string Hexa3DLagrangeP1::element_type_name() const
{
  return LibSF::library_namespace() + "." + type_name();
}

////////////////////////////////////////////////////////////////////////////////

Real Hexa3DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return volume(coord);
}

////////////////////////////////////////////////////////////////////////////////

bool Hexa3DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return in_element(coord,nodes);
}

////////////////////////////////////////////////////////////////////////////////

const ElementType::FaceConnectivity& Hexa3DLagrangeP1::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(4)(8)(12)(16)(20);
    connectivity.face_node_counts.assign(6, 4);
    connectivity.face_nodes = boost::assign::list_of(0)(3)(2)(1)
                                                    (4)(5)(6)(7)
                                                    (0)(1)(5)(4)
                                                    (1)(2)(6)(5)
                                                    (3)(7)(6)(2)
                                                    (0)(4)(7)(3);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Hexa3DLagrangeP1::face_connectivity() const
{
  return faces();
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Hexa3DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Quad3DLagrangeP1 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

bool Hexa3DLagrangeP1::in_element( const CoordsT& coord, const NodeMatrixT& nodes)
{
  
  for (Uint iFace=0; iFace<nb_faces; ++iFace)
  {
    if (!(is_orientation_inside(coord, nodes, iFace)))
      return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Hexa3DLagrangeP1::is_orientation_inside(const CoordsT& coord, const NodeMatrixT& nodes, const Uint face)
{
  //test according to http://graphics.ethz.ch/~peikert/personal/HexCellTest/
  
  const Uint a = faces().face_node_range(face)[3];
  const Uint b = faces().face_node_range(face)[2];
  const Uint c = faces().face_node_range(face)[1];
  const Uint d = faces().face_node_range(face)[0];
  
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
    RealVector ppp = T*M.inverse()*pp;
    
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

void Hexa3DLagrangeP1::shape_function(const MappedCoordsT& mapped_coord, ShapeFunctionsT& shape_func)
{
  const Real xi   = mapped_coord[0];
  const Real eta  = mapped_coord[1];
  const Real zeta = mapped_coord[2];

  const Real a1 = (1 + xi);
  const Real a2 = (1 - xi);

  const Real b1 = (1 + eta);
  const Real b2 = (1 - eta);

  const Real c1 = (1 + zeta);
  const Real c2 = (1 - zeta);

  shape_func[0] = a2*b2*c2;
  shape_func[1] = a1*b2*c2;
  shape_func[2] = a1*b1*c2;
  shape_func[3] = a2*b1*c2;
  shape_func[4] = a2*b2*c1;
  shape_func[5] = a1*b2*c1;
  shape_func[6] = a1*b1*c1;
  shape_func[7] = a2*b1*c1;

  shape_func *= 0.125;
}
  
////////////////////////////////////////////////////////////////////////////////

void Hexa3DLagrangeP1::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mapped_coord)
{  
  // Axes of the local coordinate system, centered around the centroid and going through the center of each face
  ShapeFunctionsT sf;
  shape_function(CoordsT(1.,0.,0.), sf);
  CoordsT ux = (sf*nodes).transpose();
  shape_function(CoordsT(0.,1.,0.), sf);
  CoordsT uy = (sf*nodes).transpose();
  shape_function(CoordsT(0.,0.,1.), sf);
  CoordsT uz = (sf*nodes).transpose();
  
  shape_function(CoordsT(-1.,0.,0.), sf);
  CoordsT ux_neg = (sf*nodes).transpose();
  shape_function(CoordsT(0.,-1.,0.), sf);
  CoordsT uy_neg = (sf*nodes).transpose();
  shape_function(CoordsT(0.,0.,-1.), sf);
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
    shape_function(mapped_coord, sf);
    test = (sf*nodes).transpose();
    diff = coord - test;
    test[XX] = diff.dot(nyz) * fx;  // Transform difference to the relative coordinate system and
    test[YY] = diff.dot(nxz) * fy;  // use it to adjust our initial guess
    test[ZZ] = diff.dot(nxy) * fz;
    mapped_coord += test;
    ++nb_iters;
  }
  
  if(nb_iters > 100)
    throw Common::FailedToConverge(FromHere(), "Failed to find Hexa3DLagrangeP1 mapped coordinates");
}
  
////////////////////////////////////////////////////////////////////////////////

void Hexa3DLagrangeP1::mapped_gradient(const MappedCoordsT& mapped_coord, MappedGradientT& result)
{
  const Real xi   = mapped_coord[KSI];
  const Real eta  = mapped_coord[ETA];
  const Real zeta = mapped_coord[ZTA];

  const Real a1 = (1 + xi);
  const Real a2 = (1 - xi);

  const Real b1 = (1 + eta);
  const Real b2 = (1 - eta);

  const Real c1 = (1 + zeta);
  const Real c2 = (1 - zeta);

  result(XX, 0) = -0.125 * b2*c2;
  result(YY, 0) = -0.125 * a2*c2;
  result(ZZ, 0) = -0.125 * a2*b2;

  result(XX, 1) =  0.125 * b2*c2;
  result(YY, 1) = -0.125 * a1*c2;
  result(ZZ, 1) = -0.125 * a1*b2;

  result(XX, 2) =  0.125 * b1*c2;
  result(YY, 2) =  0.125 * a1*c2;
  result(ZZ, 2) = -0.125 * a1*b1;

  result(XX, 3) = -0.125 * b1*c2;
  result(YY, 3) =  0.125 * a2*c2;
  result(ZZ, 3) = -0.125 * a2*b1;

  result(XX, 4) = -0.125 * b2*c1;
  result(YY, 4) = -0.125 * a2*c1;
  result(ZZ, 4) =  0.125 * a2*b2;

  result(XX, 5) =  0.125 * b2*c1;
  result(YY, 5) = -0.125 * a1*c1;
  result(ZZ, 5) =  0.125 * a1*b2;

  result(XX, 6) =  0.125 * b1*c1;
  result(YY, 6) =  0.125 * a1*c1;
  result(ZZ, 6) =  0.125 * a1*b1;

  result(XX, 7) = -0.125 * b1*c1;
  result(YY, 7) =  0.125 * a2*c1;
  result(ZZ, 7) =  0.125 * a2*b1;
}

////////////////////////////////////////////////////////////////////////////////

Real Hexa3DLagrangeP1::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes)
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

void Hexa3DLagrangeP1::jacobian(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result)
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

void Hexa3DLagrangeP1::jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result)
{
  JacobianT J;
  jacobian(mapped_coord, nodes, J);
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

Real Hexa3DLagrangeP1::volume(const NodeMatrixT& nodes) 
{
  return 8*jacobian_determinant(MappedCoordsT::Zero(), nodes);
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
