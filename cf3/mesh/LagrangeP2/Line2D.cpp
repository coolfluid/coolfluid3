// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP2/Line2D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Line2D>, ElementType , LibLagrangeP2 >
   Line2D_Builder(LibLagrangeP2::library_namespace()+"."+Line2D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Line2D::faces()
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

const cf3::mesh::ElementType& Line2D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<Line2D> >(Line2D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

Real Line2D::volume(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

void Line2D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid = nodes.row(2);  // The middle point of a P2 line
}

////////////////////////////////////////////////////////////////////////////////

void Line2D::compute_normal(const NodesT& nodes , CoordsT& result)
{
  /// @bug this normal is a P1 approximation
  result[XX] = 0.5*( nodes(1, YY) - nodes(0, YY));
  result[YY] = 0.5*(-nodes(1, XX) + nodes(0, XX));
  result.normalize();
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // mesh
} // cf3
