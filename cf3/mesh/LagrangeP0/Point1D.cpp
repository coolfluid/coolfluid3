// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Point1D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Point1D>, ElementType , LibLagrangeP0 >
   Point1D_Builder(LibLagrangeP0::library_namespace()+"."+Point1D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Point1D::faces()
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

const cf3::mesh::ElementType& Point1D::face_type(const cf3::Uint face)
{
  throw common::NotImplemented(FromHere(), "LagrangeP0::Point1D Does not have a face type");
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<Point1D> >(Point1D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Point1D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = nodes(0,XX);
}

////////////////////////////////////////////////////////////////////////////////

void Point1D::compute_normal(const NodesT& nodes , CoordsT& result)
{
  result[XX] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

bool Point1D::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  return (coord[XX] == nodes(0,XX));
}

////////////////////////////////////////////////////////////////////////////////

Real Point1D::volume(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Point1D::area(const NodesT& nodes)
{
  return 1.;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // mesh
} // cf3
