// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "Mesh/ElementTypeT.hpp"

#include "Mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "Mesh/LagrangeP2/Line2D.hpp"

namespace cf3 {
namespace Mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Line2D>, ElementType , LibLagrangeP2 >
   Line2D_Builder(LibLagrangeP2::library_namespace()+"."+Line2D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::Mesh::ElementType::FaceConnectivity& Line2D::faces()
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

const cf3::Mesh::ElementType& Line2D::face_type(const cf3::Uint face)
{
  static const ElementType::ConstPtr facetype( common::allocate_component<ElementTypeT<Line2D> >(Line2D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

Real Line2D::volume(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // Mesh
} // cf3
