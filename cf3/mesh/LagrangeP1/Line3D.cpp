// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "math/Consts.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Line3D.hpp"
#include "mesh/LagrangeP0/Point1D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Line3D>, ElementType , LibLagrangeP1 >
   Line3D_Builder(LibLagrangeP1::library_namespace()+"."+Line3D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Line3D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Line3D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<Line3D> >(Line3D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Line3D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = 0.5*(nodes(0,XX)+nodes(1,XX));
  centroid[YY] = 0.5*(nodes(0,YY)+nodes(1,YY));
  centroid[ZZ] = 0.5*(nodes(0,ZZ)+nodes(1,ZZ));
}

////////////////////////////////////////////////////////////////////////////////

Real Line3D::volume(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Line3D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Line3D::length(const NodesT& nodes)
{
  return (nodes.row(1)-nodes.row(0)).norm();
}

////////////////////////////////////////////////////////////////////////////////

template<>
void Line3D::compute_jacobian<Line3D::JacobianT>(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  result(KSI,XX) = 0.5*(nodes(1, XX) - nodes(0, XX));
  result(KSI,YY) = 0.5*(nodes(1, YY) - nodes(0, YY));
  result(KSI,ZZ) = 0.5*(nodes(1, ZZ) - nodes(0, ZZ));
}

////////////////////////////////////////////////////////////////////////////////

Line3D::JacobianT Line3D::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Line3D::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  const Real DxDxi = 0.5*(nodes(1, XX) - nodes(0, XX));
  const Real DyDxi = 0.5*(nodes(1, YY) - nodes(0, YY));
  const Real DzDxi = 0.5*(nodes(1, ZZ) - nodes(0, ZZ));
  return std::sqrt( DxDxi*DxDxi + DyDxi*DyDxi + DzDxi*DzDxi );
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
