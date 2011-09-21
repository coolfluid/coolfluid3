// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "Mesh/ElementTypeT.hpp"

#include "Mesh/LagrangeP3/LibLagrangeP3.hpp"
#include "Mesh/LagrangeP3/Quad2D.hpp"
#include "Mesh/LagrangeP3/Line2D.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ElementTypeT<Quad2D>, ElementType , LibLagrangeP3 >
   Quad2D_Builder(LibLagrangeP3::library_namespace()+"."+Quad2D::type_name());

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Quad2D::faces()
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

const CF::Mesh::ElementType& Quad2D::face_type(const CF::Uint face)
{
  static const ElementType::ConstPtr facetype( Common::allocate_component<ElementTypeT<LagrangeP3::Line2D> >(LagrangeP3::Line2D::type_name()) );
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
} // Mesh
} // CF
