// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "math/Consts.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Line1D.hpp"
#include "mesh/LagrangeP0/Point1D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Line1D>, ElementType , LibLagrangeP1 >
   Line1D_Builder(LibLagrangeP1::library_namespace()+"."+Line1D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Line1D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0)(1);
    connectivity.stride.assign(nb_faces, 1);
    connectivity.nodes = boost::assign::list_of(0)
                                               (1);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Line1D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<LagrangeP0::Point1D> >(LagrangeP0::Point1D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Line1D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  cf3_assert(nodes.rows()==2);
  cf3_assert(nodes.cols()==1);
  cf3_assert(centroid.size()==1);
  centroid[0] = 0.5*(nodes(0,XX)+nodes(1,XX));
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

////////////////////////////////////////////////////////////////////////////////

void Line1D::compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord)
{
  const Real& x0 = nodes(0, XX);
  const Real& x1 = nodes(1, XX);
  mapped_coord[KSI] = (2*coord[XX] - (x1 + x0)) / (x1 - x0);
}

////////////////////////////////////////////////////////////////////////////////

Line1D::MappedCoordsT Line1D::mapped_coordinate(const CoordsT& coord, const NodesT& nodes)
{
  MappedCoordsT result;
  compute_mapped_coordinate(coord,nodes,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Line1D::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes) {
  return 0.5*volume(nodes);
}

////////////////////////////////////////////////////////////////////////////////

void Line1D::compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  cf3_assert(result.rows()==dimension);
  cf3_assert(result.cols()==dimension);
  result(KSI,XX) = jacobian_determinant(mapped_coord, nodes);
}

////////////////////////////////////////////////////////////////////////////////

Line1D::JacobianT Line1D::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

void Line1D::compute_jacobian_adjoint(const MappedCoordsT& mappedCoord, const NodesT& nodes, JacobianT& result)
{
  result(KSI,XX) = 1.;
}

////////////////////////////////////////////////////////////////////////////////

Real Line1D::volume(const NodesT& nodes)
{
  return std::abs(nodes(1, XX) - nodes(0, XX));
}

////////////////////////////////////////////////////////////////////////////////

Real Line1D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Line1D::CoordsT Line1D::plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation)
{
  CoordsT result;
  compute_plane_jacobian_normal(mapped_coord,nodes,orientation,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

void Line1D::compute_plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation, CoordsT& result)
{
  result[XX] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
