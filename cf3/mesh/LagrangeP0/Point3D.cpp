// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Point3D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Point3D>, ElementType , LibLagrangeP0 >
   Point3D_Builder(LibLagrangeP0::library_namespace()+"."+Point3D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Point3D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0);
    connectivity.stride.assign(1, 1);
    connectivity.nodes = boost::assign::list_of(0);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Point3D::face_type(const cf3::Uint face)
{
  throw common::NotImplemented(FromHere(), "LagrangeP0::Point3D Does not have a face type");
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<Point3D> >(Point3D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Point3D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = nodes(0,XX);
  centroid[YY] = nodes(0,YY);
  centroid[ZZ] = nodes(0,ZZ);
}

////////////////////////////////////////////////////////////////////////////////

bool Point3D::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  return (coord[XX] == nodes(0,XX) && coord[YY] == nodes(0,YY) && coord[ZZ] == nodes(0,ZZ));
}

////////////////////////////////////////////////////////////////////////////////

Real Point3D::volume(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Point3D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // mesh
} // cf3
