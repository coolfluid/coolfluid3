// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "math/Consts.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Prism3D.hpp"
#include "mesh/LagrangeP1/Quad3D.hpp"
#include "mesh/LagrangeP1/Triag3D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Prism3D>, ElementType , LibLagrangeP1 >
   Prism3D_Builder(LibLagrangeP1::library_namespace()+"."+Prism3D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Prism3D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0)(3)(6)(10)(14)(18);
    connectivity.stride = boost::assign::list_of(3)(3)(4)(4)(4)(4);
    connectivity.nodes = boost::assign::list_of
        (0)(1)(2)
        (3)(5)(4)
        (0)(2)(5)(3)
        (0)(3)(4)(1)
        (2)(1)(4)(5);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Prism3D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > quad( common::allocate_component<ElementTypeT<LagrangeP1::Quad3D> >(LagrangeP1::Quad3D::type_name()) );
  static const boost::shared_ptr< ElementType > triag( common::allocate_component<ElementTypeT<LagrangeP1::Triag3D> >(LagrangeP1::Triag3D::type_name()) );
  if(face < 2)
    return *triag;

  return *quad;
}

////////////////////////////////////////////////////////////////////////////////

void Prism3D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = nodes(0,XX)+nodes(1,XX)+nodes(2,XX)+nodes(3,XX)+nodes(4,XX)+nodes(5,XX);
  centroid[YY] = nodes(0,YY)+nodes(1,YY)+nodes(2,YY)+nodes(3,YY)+nodes(4,YY)+nodes(5,YY);
  centroid[ZZ] = nodes(0,ZZ)+nodes(1,ZZ)+nodes(2,ZZ)+nodes(3,ZZ)+nodes(4,ZZ)+nodes(5,ZZ);
  centroid /= 6.;
}

////////////////////////////////////////////////////////////////////////////////

bool Prism3D::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  throw common::NotImplemented(FromHere(), "is_coord_in_element is not implemented for Prism3D");
}

////////////////////////////////////////////////////////////////////////////////

void Prism3D::compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord)
{
  throw common::NotImplemented(FromHere(), "compute_mapped_coordinate is not implemented for Prism3D");
}

////////////////////////////////////////////////////////////////////////////////

Prism3D::MappedCoordsT Prism3D::mapped_coordinate(const CoordsT& coord, const NodesT& nodes)
{
  throw common::NotImplemented(FromHere(), "mapped_coordinate is not implemented for Prism3D");
}

////////////////////////////////////////////////////////////////////////////////

template <>
void Prism3D::compute_jacobian<Prism3D::JacobianT>(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  const Real ksi = mapped_coord[KSI]; const Real eta = mapped_coord[ETA]; const Real zta = mapped_coord[ZTA];

  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);
  const Real x3 = nodes(3, XX);
  const Real x4 = nodes(4, XX);
  const Real x5 = nodes(5, XX);

  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);
  const Real y3 = nodes(3, YY);
  const Real y4 = nodes(4, YY);
  const Real y5 = nodes(5, YY);

  const Real z0 = nodes(0, ZZ);
  const Real z1 = nodes(1, ZZ);
  const Real z2 = nodes(2, ZZ);
  const Real z3 = nodes(3, ZZ);
  const Real z4 = nodes(4, ZZ);
  const Real z5 = nodes(5, ZZ);

  result(KSI,XX) = -x0*(-0.5*zta + 0.5) + x1*(-0.5*zta + 0.5) - x3*(0.5*zta + 0.5) + x4*(0.5*zta + 0.5);
  result(KSI,YY) = -y0*(-0.5*zta + 0.5) + y1*(-0.5*zta + 0.5) - y3*(0.5*zta + 0.5) + y4*(0.5*zta + 0.5);
  result(KSI,ZZ) = -z0*(-0.5*zta + 0.5) + z1*(-0.5*zta + 0.5) - z3*(0.5*zta + 0.5) + z4*(0.5*zta + 0.5);
  result(ETA,XX) = -x0*(-0.5*zta + 0.5) + x2*(-0.5*zta + 0.5) - x3*(0.5*zta + 0.5) + x5*(0.5*zta + 0.5);
  result(ETA,YY) = -y0*(-0.5*zta + 0.5) + y2*(-0.5*zta + 0.5) - y3*(0.5*zta + 0.5) + y5*(0.5*zta + 0.5);
  result(ETA,ZZ) = -z0*(-0.5*zta + 0.5) + z2*(-0.5*zta + 0.5) - z3*(0.5*zta + 0.5) + z5*(0.5*zta + 0.5);
  result(ZTA,XX) = -0.5*eta*x2 + 0.5*eta*x5 - 0.5*ksi*x1 + 0.5*ksi*x4 - 0.5*x0*(-eta - ksi + 1) + 0.5*x3*(-eta - ksi + 1);
  result(ZTA,YY) = -0.5*eta*y2 + 0.5*eta*y5 - 0.5*ksi*y1 + 0.5*ksi*y4 - 0.5*y0*(-eta - ksi + 1) + 0.5*y3*(-eta - ksi + 1);
  result(ZTA,ZZ) = -0.5*eta*z2 + 0.5*eta*z5 - 0.5*ksi*z1 + 0.5*ksi*z4 - 0.5*z0*(-eta - ksi + 1) + 0.5*z3*(-eta - ksi + 1);
}

////////////////////////////////////////////////////////////////////////////////

void Prism3D::compute_jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  JacobianT jac;
  compute_jacobian(mapped_coord, nodes, jac);
  Real det;
  bool is_invertible;
  jac.computeInverseAndDetWithCheck(result, det, is_invertible);
  cf3_assert(is_invertible);
  result *= det;
}

////////////////////////////////////////////////////////////////////////////////

Real Prism3D::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT jac;
  compute_jacobian(mapped_coord, nodes, jac);
  return jac.determinant();
}

////////////////////////////////////////////////////////////////////////////////

Prism3D::JacobianT Prism3D::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Prism3D::volume(const NodesT& nodes)
{
  MappedCoordsT center;
  center[KSI] = 0.3333333333333333333333333;
  center[ETA] = 0.3333333333333333333333333;
  center[ZTA] = 0.;
  return jacobian_determinant(center, nodes);
}

////////////////////////////////////////////////////////////////////////////////

Real Prism3D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
