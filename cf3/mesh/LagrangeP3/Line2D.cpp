// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "mesh/ElementTypeT.hpp"
#include "mesh/ShapeFunctionT.hpp"

#include "mesh/LagrangeP3/LibLagrangeP3.hpp"
#include "mesh/LagrangeP3/Line2D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Line2D>, ElementType , LibLagrangeP3 >
   Line2D_Builder(LibLagrangeP3::library_namespace()+"."+Line2D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Line2D::faces()
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

} // LagrangeP3
} // mesh
} // cf3
