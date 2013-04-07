// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP2/Quad3D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Quad3D>, ElementType , LibLagrangeP2 >
   Quad3D_Builder(LibLagrangeP2::library_namespace()+"."+Quad3D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Quad3D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0);
    connectivity.stride.assign(1, nb_nodes);
    connectivity.nodes = boost::assign::list_of(0)(1)(2)(3);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Quad3D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<LagrangeP2::Quad3D> >(LagrangeP2::Quad3D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Quad3D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = 0.25*(nodes(0,XX)+nodes(1,XX)+nodes(2,XX)+nodes(3,XX));
  centroid[YY] = 0.25*(nodes(0,YY)+nodes(1,YY)+nodes(2,YY)+nodes(3,YY));
  centroid[ZZ] = 0.25*(nodes(0,ZZ)+nodes(1,ZZ)+nodes(2,ZZ)+nodes(3,ZZ));
}

////////////////////////////////////////////////////////////////////////////////

void Quad3D::compute_normal(const NodesT& nodes , CoordsT& normal)
{
  JacobianT jac = jacobian(MappedCoordsT::Zero(), nodes);

  normal[XX] = jac(KSI,YY)*jac(ETA,ZZ) - jac(KSI,ZZ)*jac(ETA,YY);
  normal[YY] = jac(KSI,ZZ)*jac(ETA,XX) - jac(KSI,XX)*jac(ETA,ZZ);
  normal[ZZ] = jac(KSI,XX)*jac(ETA,YY) - jac(KSI,YY)*jac(ETA,XX);

  normal.normalize();
}

////////////////////////////////////////////////////////////////////////////////

template <>
void Quad3D::compute_jacobian<Quad3D::JacobianT>(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  JacobianCoefficients jc(nodes);

  const Real xi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  result(KSI,XX) = jc.bx + jc.dx*eta;
  result(KSI,YY) = jc.by + jc.dy*eta;
  result(KSI,ZZ) = jc.bz + jc.dz*eta;

  result(ETA,XX) = jc.cx + jc.dx*xi;
  result(ETA,YY) = jc.cy + jc.dy*xi;
  result(ETA,ZZ) = jc.cz + jc.dz*xi;
}

////////////////////////////////////////////////////////////////////////////////

Quad3D::JacobianT Quad3D::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad3D::volume(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad3D::area(const NodesT& nodes)
{
  JacobianT jac = jacobian(MappedCoordsT::Zero(), nodes);
  CoordsT face_normal;
  face_normal[XX] = jac(KSI,YY)*jac(ETA,ZZ) - jac(KSI,ZZ)*jac(ETA,YY);
  face_normal[YY] = jac(KSI,ZZ)*jac(ETA,XX) - jac(KSI,XX)*jac(ETA,ZZ);
  face_normal[ZZ] = jac(KSI,XX)*jac(ETA,YY) - jac(KSI,YY)*jac(ETA,XX);
  return 4.*face_normal.norm();
}

////////////////////////////////////////////////////////////////////////////////

void Quad3D::normal(const MappedCoordsT& mapped_coord, const NodesT& nodes , CoordsT& normal)
{
  JacobianT jac = jacobian(MappedCoordsT::Zero(), nodes);

  normal[XX] = jac(KSI,YY)*jac(ETA,ZZ) - jac(KSI,ZZ)*jac(ETA,YY);
  normal[YY] = jac(KSI,ZZ)*jac(ETA,XX) - jac(KSI,XX)*jac(ETA,ZZ);
  normal[ZZ] = jac(KSI,XX)*jac(ETA,YY) - jac(KSI,YY)*jac(ETA,XX);
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // mesh
} // cf3
