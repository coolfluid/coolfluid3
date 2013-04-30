// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP3/LibLagrangeP3.hpp"
#include "mesh/LagrangeP3/Quad2D.hpp"
#include "mesh/LagrangeP3/Line2D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Quad2D>, ElementType , LibLagrangeP3 >
   Quad2D_Builder(LibLagrangeP3::library_namespace()+"."+Quad2D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Quad2D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0)(4)(8)(12);
    connectivity.stride.assign(nb_faces, 4);
    connectivity.nodes = boost::assign::list_of(0)(4)(5)(1)
                                               (1)(6)(7)(2)
                                               (2)(8)(9)(3)
                                               (3)(10)(11)(0);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Quad2D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<LagrangeP3::Line2D> >(LagrangeP3::Line2D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid = (SF::value(MappedCoordsT::Zero()) * nodes ).transpose();
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP3
} // mesh
} // cf3
