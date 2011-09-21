// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "Math/Consts.hpp"

#include "Mesh/ElementTypeT.hpp"

#include "Mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "Mesh/LagrangeP2/Quad2D.hpp"
#include "Mesh/LagrangeP2/Line2D.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ElementTypeT<Quad2D>, ElementType , LibLagrangeP2 >
   Quad2D_Builder(LibLagrangeP2::library_namespace()+"."+Quad2D::type_name());

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Quad2D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0)(3)(6)(9);
    connectivity.stride.assign(nb_faces, 3);
    connectivity.nodes = boost::assign::list_of(0)(1)(4)
                                               (1)(2)(5)
                                               (2)(3)(6)
                                               (3)(0)(7);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Quad2D::face_type(const CF::Uint face)
{
  static const ElementType::ConstPtr facetype( Common::allocate_component<ElementTypeT<LagrangeP2::Line2D> >(LagrangeP2::Line2D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid = nodes.row(8);
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2D::volume(const NodesT& nodes)
{
  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);
  const Real x3 = nodes(3, XX);
  const Real x4 = nodes(4, XX);
  const Real x5 = nodes(5, XX);
  const Real x6 = nodes(6, XX);
  const Real x7 = nodes(7, XX);
  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);
  const Real y3 = nodes(3, YY);
  const Real y4 = nodes(4, YY);
  const Real y5 = nodes(5, YY);
  const Real y6 = nodes(6, YY);
  const Real y7 = nodes(7, YY);

  return (4*((x7 - x4)*y0 + (x4 - x5)*y1 + (x5 - x6)*y2 + (x6 - x7)*y3) +
          x1*(y0 - y2 - 4*y4 + 4*y5) + x2*(y1 - y3 - 4*y5 + 4*y6) +
          x0*(y3 - y1 + 4*y4 - 4*y7) + x3*(y2 - y0 - 4*y6 + 4*y7)
         )/6.;

}

////////////////////////////////////////////////////////////////////////////////

Real Quad2D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // Mesh
} // CF
