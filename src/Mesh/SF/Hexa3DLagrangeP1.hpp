// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Hexa3DLagrangeP1_hpp
#define CF_Mesh_SF_Hexa3DLagrangeP1_hpp

#include "Math/RealMatrix.hpp"
#include "Math/MathFunctions.hpp"
#include "Math/MatrixInverterT.hpp"

#include "Mesh/Hexa3D.hpp"
#include "Mesh/ElementNodes.hpp"

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
struct SF_API Hexa3DLagrangeP1  : public Hexa3D {

/// Compute the shape functions corresponding to the given
/// mapped coordinates
/// @param mappedCoord The mapped coordinates
/// @param shapeFunc Vector storing the result
static void shape_function(const RealVector& mapped_coord, RealVector& shape_func)
{
  cf_assert(shape_func.size() == nb_nodes);
  cf_assert(mapped_coord.size() == dimension);

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
  
/// Compute Mapped Coordinates
/// @param coord contains the coordinates to be mapped
/// @param nodes contains the nodes
/// @param mappedCoord Store the output mapped coordinates
template<typename NodesT>
static void mapped_coordinates(const RealVector& coord, const NodesT& nodes, RealVector& mapped_coord)
{  
  // Axes of the local coordinate system, centered around the centroid and going through the center of each face
  RealVector ux(0.,3);
  RealVector uy(0.,3);
  RealVector uz(0.,3);
  eval<Hexa3DLagrangeP1>(point3(1.,0.,0.), nodes, ux);
  eval<Hexa3DLagrangeP1>(point3(0.,1.,0.), nodes, uy);
  eval<Hexa3DLagrangeP1>(point3(0.,0.,1.), nodes, uz);
  
  RealVector ux_neg(0.,3);
  RealVector uy_neg(0.,3);
  RealVector uz_neg(0.,3);
  eval<Hexa3DLagrangeP1>(point3(-1.,0.,0.), nodes, ux_neg);
  eval<Hexa3DLagrangeP1>(point3(0.,-1.,0.), nodes, uy_neg);
  eval<Hexa3DLagrangeP1>(point3(0.,0.,-1.), nodes, uz_neg);
  
  RealVector centroid(0., 3);
  centroid[XX] = (ux[XX] + ux_neg[XX]) * 0.5;
  centroid[YY] = (uy[YY] + uy_neg[YY]) * 0.5;
  centroid[ZZ] = (uz[ZZ] + uz_neg[ZZ]) * 0.5;
  
  ux -= ux_neg;
  uy -= uy_neg;
  uz -= uz_neg;
  
  ux *= 0.5; // because the origin is at the center
  uy *= 0.5;
  uz *= 0.5;
  
  const Real ux_len_inv = 1. / ux.norm2();
  const Real uy_len_inv = 1. / uy.norm2();
  const Real uz_len_inv = 1. / uz.norm2();
  
  ux *= ux_len_inv;
  uy *= uy_len_inv;
  uz *= uz_len_inv;
  
  // Normal vectors
  RealVector nyz(3);
  RealVector nxz(3);
  RealVector nxy(3);
  Math::MathFunctions::crossProd(uy, uz, nyz);
  Math::MathFunctions::crossProd(ux, uz, nxz);
  Math::MathFunctions::crossProd(ux, uy, nxy);
  
  // division factors for line-plane intersection
  const Real fx = ux_len_inv / Math::MathFunctions::innerProd(ux, nyz);
  const Real fy = uy_len_inv / Math::MathFunctions::innerProd(uy, nxz);
  const Real fz = uz_len_inv / Math::MathFunctions::innerProd(uz, nxy);
  
  RealVector diff(3);
  diff = coord-centroid;
  RealVector test(3);
  const Real threshold = 1e-24; // 1e-12 squared, because we compare the squared distance
  Uint nb_iters = 0;
  // Initial guess will be correct if our element is a parallelepiped
  mapped_coord[KSI] = Math::MathFunctions::innerProd(diff, nyz) * fx;
  mapped_coord[ETA] = Math::MathFunctions::innerProd(diff, nxz) * fy;
  mapped_coord[ZTA] = Math::MathFunctions::innerProd(diff, nxy) * fz;
  while (nb_iters < 100 && Math::MathFunctions::innerProd(diff, diff) > threshold)
  {
    eval<Hexa3DLagrangeP1>(mapped_coord, nodes, test);
    diff = coord - test;
    test[XX] = Math::MathFunctions::innerProd(diff, nyz) * fx;  // Transform difference to the relative coordinate system and
    test[YY] = Math::MathFunctions::innerProd(diff, nxz) * fy;  // use it to adjust our initial guess
    test[ZZ] = Math::MathFunctions::innerProd(diff, nxy) * fz;
    mapped_coord += test;
    ++nb_iters;
  }
  
  if(nb_iters > 100)
    throw Common::ConvergenceNotReached(FromHere(), "Failed to find Hexa3DLagrangeP1 mapped coordinates");
}
  

/// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
/// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
/// coordinates.
/// @param mapped_coord The mapped coordinates where the gradient should be calculated
/// @param result Storage for the resulting gradient matrix
static void mapped_gradient(const RealVector& mapped_coord, RealMatrix& result)
{
  cf_assert(result.nbCols() == nb_nodes);
  cf_assert(result.nbRows() == dimension);

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

/// Compute the jacobian determinant at the given mapped coordinates
template<typename NodesT>
static Real jacobian_determinant(const RealVector& mapped_coord, const NodesT& nodes)
{
  cf_assert(mapped_coord.size() == dimension);
  cf_assert(nodes.size() == nb_nodes);

  const Real x0 = nodes[0][XX];
  const Real x1 = nodes[1][XX];
  const Real x2 = nodes[2][XX];
  const Real x3 = nodes[3][XX];
  const Real x4 = nodes[4][XX];
  const Real x5 = nodes[5][XX];
  const Real x6 = nodes[6][XX];
  const Real x7 = nodes[7][XX];

  const Real y0 = nodes[0][YY];
  const Real y1 = nodes[1][YY];
  const Real y2 = nodes[2][YY];
  const Real y3 = nodes[3][YY];
  const Real y4 = nodes[4][YY];
  const Real y5 = nodes[5][YY];
  const Real y6 = nodes[6][YY];
  const Real y7 = nodes[7][YY];

  const Real z0 = nodes[0][ZZ];
  const Real z1 = nodes[1][ZZ];
  const Real z2 = nodes[2][ZZ];
  const Real z3 = nodes[3][ZZ];
  const Real z4 = nodes[4][ZZ];
  const Real z5 = nodes[5][ZZ];
  const Real z6 = nodes[6][ZZ];
  const Real z7 = nodes[7][ZZ];

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

/// Compute the Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
template<typename NodesT>
static void jacobian(const RealVector& mapped_coord, const NodesT& nodes, RealMatrix& result)
{
  cf_assert(result.nbRows() == dimensionality);
  cf_assert(result.nbCols() == dimension);

  const Real xi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];
  const Real zeta = mapped_coord[ZTA];

  const Real x0 = nodes[0][XX];
  const Real x1 = nodes[1][XX];
  const Real x2 = nodes[2][XX];
  const Real x3 = nodes[3][XX];
  const Real x4 = nodes[4][XX];
  const Real x5 = nodes[5][XX];
  const Real x6 = nodes[6][XX];
  const Real x7 = nodes[7][XX];

  const Real y0 = nodes[0][YY];
  const Real y1 = nodes[1][YY];
  const Real y2 = nodes[2][YY];
  const Real y3 = nodes[3][YY];
  const Real y4 = nodes[4][YY];
  const Real y5 = nodes[5][YY];
  const Real y6 = nodes[6][YY];
  const Real y7 = nodes[7][YY];

  const Real z0 = nodes[0][ZZ];
  const Real z1 = nodes[1][ZZ];
  const Real z2 = nodes[2][ZZ];
  const Real z3 = nodes[3][ZZ];
  const Real z4 = nodes[4][ZZ];
  const Real z5 = nodes[5][ZZ];
  const Real z6 = nodes[6][ZZ];
  const Real z7 = nodes[7][ZZ];

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

/// Compute the adjoint of Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting adjoint
template<typename NodesT>
static void jacobian_adjoint(const RealVector& mapped_coord, const NodesT& nodes, RealMatrix& result) {
  cf_assert(result.nbRows() == dimensionality);
  cf_assert(result.nbCols() == dimension);
  RealMatrix jac(dimensionality,dimension);
  jacobian(mapped_coord, nodes, jac);

  result[0] =  (jac[4]*jac[8] - jac[5]*jac[7]);
  result[1] = -(jac[1]*jac[8] - jac[2]*jac[7]);
  result[2] =  (jac[1]*jac[5] - jac[4]*jac[2]);
  result[3] = -(jac[3]*jac[8] - jac[5]*jac[6]);
  result[4] =  (jac[0]*jac[8] - jac[2]*jac[6]);
  result[5] = -(jac[0]*jac[5] - jac[2]*jac[3]);
  result[6] =  (jac[3]*jac[7] - jac[4]*jac[6]);
  result[7] = -(jac[0]*jac[7] - jac[1]*jac[6]);
  result[8] =  (jac[0]*jac[4] - jac[1]*jac[3]);
}

/// Volume of the cell
template<typename NodesT>
static Real volume(const NodesT& nodes) {
  RealVector center(0.0, 3); // center in mapped coords
  return 8*jacobian_determinant(center, nodes);
}
	
//template<typename NodesT>
static bool in_element(const RealVector& coord, const ElementType::NodesT& nodes);

/// Number of nodes
static const Uint nb_nodes = 8;

/// Order of the shape function
static const Uint order = 1;

/// Indices for the faces in each direction
enum FaceNumbering { ZNEG, ZPOS, YNEG, XPOS, YPOS, XNEG};

static const FaceConnectivity& faces();

Hexa3DLagrangeP1();
virtual std::string getElementTypeName() const;
virtual Real computeVolume(const NodesT& coord) const;
virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;
virtual const FaceConnectivity& face_connectivity() const;
virtual const ElementType& face_type(const Uint face) const;
virtual Real jacobian_determinantV ( const CF::RealVector& mapped_coord, const CF::Mesh::ElementType::NodesT& nodes ) const;

private:
	
/// @return if coordinate is oriented towards the inside of the element from the point of view from a given face
/// @param coord [in]  coordinates
/// @param nodes [in]  the nodes defining the element
/// @param face  [in]  the face number fo the element
static	bool is_orientation_inside(const RealVector& coord, const NodesT& nodes, const Uint face);
	
};

} // namespace SF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_SF_Hexa3DLagrangeP1 */
