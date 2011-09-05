// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/ElementTypeT.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

ElementTypeFallBack::MappedCoordsT ElementTypeFallBack::mapped_coordinate(const CoordsT& coord, const NodesT& nodes)
{
  throw Common::NotImplemented(FromHere(),"mapped_coordinate not implemented or not applicable.\n Check backtrace to see which element type this is about.");
  return MappedCoordsT();
}

void ElementTypeFallBack::compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord)
{
  throw Common::NotImplemented(FromHere(),"compute_mapped_coordinate not implemented or not applicable.\n Check backtrace to see which element type this is about.");
}

Real ElementTypeFallBack::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  throw Common::NotImplemented(FromHere(),"jacobian_determinant not implemented or not applicable.\n Check backtrace to see which element type this is about.");
  return 0.;
}

ElementTypeFallBack::JacobianT ElementTypeFallBack::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  throw Common::NotImplemented(FromHere(),"jacobian not implemented or not applicable.\n Check backtrace to see which element type this is about.");
  return JacobianT();
}

void ElementTypeFallBack::compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& jacobian)
{
  throw Common::NotImplemented(FromHere(),"compute_jacobian not implemented or not applicable.\n Check backtrace to see which element type this is about.");
}

void ElementTypeFallBack::compute_jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  throw Common::NotImplemented(FromHere(),"compute_jacobian_adjoint not implemented or not applicable.\n Check backtrace to see which element type this is about.");
}

Real ElementTypeFallBack::volume(const NodesT& nodes)
{
  throw Common::NotImplemented(FromHere(),"volume not implemented or not applicable.\n Check backtrace to see which element type this is about.");
  return 0.;
}

Real ElementTypeFallBack::area(const NodesT& nodes)
{
  throw Common::NotImplemented(FromHere(),"area not implemented or not applicable.\n Check backtrace to see which element type this is about.");
  return 0.;
}

void ElementTypeFallBack::compute_normal(const NodesT& nodes, CoordsT& normal)
{
  throw Common::NotImplemented(FromHere(),"compute_normal not implemented or not applicable.\n Check backtrace to see which element type this is about.");
}

void ElementTypeFallBack::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  throw Common::NotImplemented(FromHere(),"compute_centroid not implemented or not applicable.\n Check backtrace to see which element type this is about.");
}

bool ElementTypeFallBack::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  throw Common::NotImplemented(FromHere(),"is_coord_in_element not implemented or not applicable.\n Check backtrace to see which element type this is about.");
  return false;
}

ElementTypeFallBack::CoordsT ElementTypeFallBack::plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation)
{
  throw Common::NotImplemented(FromHere(),"plane_jacobian_normal not implemented or not applicable.\n Check backtrace to see which element type this is about.");
  return CoordsT();
}

void ElementTypeFallBack::compute_plane_jacobian_normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, const CoordRef orientation, CoordsT& result)
{
  throw Common::NotImplemented(FromHere(),"compute_plane_jacobian_normal not implemented or not applicable.\n Check backtrace to see which element type this is about.");
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
