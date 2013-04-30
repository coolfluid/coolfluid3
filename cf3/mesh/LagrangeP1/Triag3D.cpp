// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Triag3D.hpp"
#include "mesh/LagrangeP1/Line2D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Triag3D>, ElementType , LibLagrangeP1 >
   Triag3D_Builder(LibLagrangeP1::library_namespace()+"."+Triag3D::type_name());

////////////////////////////////////////////////////////////////////////////////


/// Stand-alone helper function for reuse in volume() and jacobian_determinant()
Real jacobian_determinant_helper(const Triag3D::NodesT& nodes)
{
  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);
  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);
  return (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Triag3D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0);
    connectivity.stride.assign(1, nb_nodes);
    connectivity.nodes = boost::assign::list_of(0)(1)(2);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Triag3D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<Triag3D> >(Triag3D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Triag3D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = nodes(0,XX)+nodes(1,XX)+nodes(2,XX);
  centroid[YY] = nodes(0,YY)+nodes(1,YY)+nodes(2,YY);
  centroid[ZZ] = nodes(0,ZZ)+nodes(1,ZZ)+nodes(2,ZZ);
  centroid /= 3.;
}

////////////////////////////////////////////////////////////////////////////////

void Triag3D::compute_normal(const NodesT& nodes, CoordsT& result)
{
  /// @todo this could be simpler for this application
  /// Jacobian could be avoided
  JacobianT jac = jacobian(MappedCoordsT::Zero(),nodes);

  result[XX] = jac(KSI,YY)*jac(ETA,ZZ) - jac(KSI,ZZ)*jac(ETA,YY);
  result[YY] = jac(KSI,ZZ)*jac(ETA,XX) - jac(KSI,XX)*jac(ETA,ZZ);
  result[ZZ] = jac(KSI,XX)*jac(ETA,YY) - jac(KSI,YY)*jac(ETA,XX);

  // turn into unit vector
  result.normalize();
}

////////////////////////////////////////////////////////////////////////////////

template <>
void Triag3D::compute_jacobian<Triag3D::JacobianT>(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);

  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);

  const Real z0 = nodes(0, ZZ);
  const Real z1 = nodes(1, ZZ);
  const Real z2 = nodes(2, ZZ);

  result(KSI,XX) = x1 - x0;
  result(KSI,YY) = y1 - y0;
  result(KSI,ZZ) = z1 - z0;

  result(ETA,XX) = x2 - x0;
  result(ETA,YY) = y2 - y0;
  result(ETA,ZZ) = z2 - z0;
}

////////////////////////////////////////////////////////////////////////////////

Triag3D::JacobianT Triag3D::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Triag3D::volume(const NodesT& nodes)
{
  return 0.5 * jacobian_determinant_helper(nodes);
}

////////////////////////////////////////////////////////////////////////////////

Real Triag3D::area(const NodesT& nodes)
{
  JacobianT jac = jacobian(MappedCoordsT::Zero(),nodes);
  CoordsT face_normal;
  face_normal[XX] = jac(KSI,YY)*jac(ETA,ZZ) - jac(KSI,ZZ)*jac(ETA,YY);
  face_normal[YY] = jac(KSI,ZZ)*jac(ETA,XX) - jac(KSI,XX)*jac(ETA,ZZ);
  face_normal[ZZ] = jac(KSI,XX)*jac(ETA,YY) - jac(KSI,YY)*jac(ETA,XX);

  return 0.5*face_normal.norm();
}

////////////////////////////////////////////////////////////////////////////////

void Triag3D::normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, CoordsT& result)
{
  JacobianT jac = jacobian(MappedCoordsT::Zero(),nodes);

  result[XX] = jac(KSI,YY)*jac(ETA,ZZ) - jac(KSI,ZZ)*jac(ETA,YY);
  result[YY] = jac(KSI,ZZ)*jac(ETA,XX) - jac(KSI,XX)*jac(ETA,ZZ);
  result[ZZ] = jac(KSI,XX)*jac(ETA,YY) - jac(KSI,YY)*jac(ETA,XX);
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
