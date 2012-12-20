// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP2/Line1D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Line1D>, ElementType , LibLagrangeP2 >
   Line1D_Builder(LibLagrangeP2::library_namespace()+"."+Line1D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Line1D::faces()
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

const cf3::mesh::ElementType& Line1D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<Line1D> >(Line1D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Line1D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid = nodes.row(2);  // The middle point of a P2 line
}

////////////////////////////////////////////////////////////////////////////////

bool Line1D::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  static const Real tolerance = 1e-12;
  MappedCoordsT mapped_coord = mapped_coordinate(coord,nodes);
  if( (mapped_coord[KSI] >= -0.5 - tolerance) &&
    (mapped_coord[KSI] <= 0.5 + tolerance) )
  {
    return true;
  }
  else
  {
    return false;
  }
}

void Line1D::compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord)
{
  throw common::NotImplemented(FromHere(), "Inverse mapped coord computation not implemented for LagrangeP2::Line1D");
}

Line1D::MappedCoordsT Line1D::mapped_coordinate(const CoordsT& coord, const NodesT& nodes)
{
  MappedCoordsT result;
  compute_mapped_coordinate(coord,nodes,result);
  return result;
}

Real Line1D::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  return (mapped_coord[KSI]-0.5)*nodes[0] + (mapped_coord[KSI]+0.5)*nodes[1] + 2.*mapped_coord[KSI]*nodes[2];
}

void Line1D::compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  cf3_assert(result.rows()==dimension);
  cf3_assert(result.cols()==dimension);
  result(KSI,XX) = jacobian_determinant(mapped_coord, nodes);
}

Real Line1D::volume(const NodesT& nodes)
{
  return nodes[1] - nodes[0];
}

Real Line1D::area(const ElementTypeBase< Line1D, Line1D_traits >::NodesT& nodes)
{
  return 0;
}

void Line1D::compute_plane_jacobian_normal(const ElementTypeBase< Line1D, Line1D_traits >::MappedCoordsT& mapped_coord, const ElementTypeBase< Line1D, Line1D_traits >::NodesT& nodes, const CoordRef orientation, ElementTypeBase< Line1D, Line1D_traits >::CoordsT& result)
{
  throw common::NotImplemented(FromHere(), "compute_plane_jacobian_normal not implemented for LagrangeP2::Line1D");
}

ElementTypeBase< Line1D, Line1D_traits >::JacobianT Line1D::jacobian(const ElementTypeBase< Line1D, Line1D_traits >::MappedCoordsT& mapped_coord, const ElementTypeBase< Line1D, Line1D_traits >::NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
}

ElementTypeBase< Line1D, Line1D_traits >::CoordsT Line1D::plane_jacobian_normal(const ElementTypeBase< Line1D, Line1D_traits >::MappedCoordsT& mapped_coord, const ElementTypeBase< Line1D, Line1D_traits >::NodesT& nodes, const CoordRef orientation)
{
  throw common::NotImplemented(FromHere(), "plane_jacobian_normal not implemented for LagrangeP2::Line1D");
}




} // LagrangeP2
} // mesh
} // cf3
