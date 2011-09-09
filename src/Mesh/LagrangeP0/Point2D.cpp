// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "Mesh/ElementTypeT.hpp"

#include "Mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "Mesh/LagrangeP0/Point2D.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ElementTypeT<Point2D>, ElementType , LibLagrangeP0 >
   Point2D_Builder(LibLagrangeP0::library_namespace()+"."+Point2D::type_name());

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Point2D::faces()
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

const CF::Mesh::ElementType& Point2D::face_type(const CF::Uint face)
{
  throw Common::NotImplemented(FromHere(), "LagrangeP0::Point2D Does not have a face type");
  static const ElementTypeT<Point2D> facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Point2D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = nodes(0,XX);
  centroid[YY] = nodes(0,YY);
}

////////////////////////////////////////////////////////////////////////////////

bool Point2D::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  return (coord[XX] == nodes(0,XX) && coord[YY] == nodes(0,YY));
}

////////////////////////////////////////////////////////////////////////////////

Real Point2D::volume(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Point2D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // Mesh
} // CF
