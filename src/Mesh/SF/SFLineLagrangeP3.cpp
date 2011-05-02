// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "SFLineLagrangeP3.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SFLineLagrangeP3, ShapeFunction, LibSF > SFLineLagrangeP3_Builder;

////////////////////////////////////////////////////////////////////////////////

SFLineLagrangeP3::SFLineLagrangeP3(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void SFLineLagrangeP3::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real onesixteenth = 1.0/16.0;

  result[0] = -onesixteenth*(1.0 - mapped_coord[KSI])*(1.0 - 9.0*mapped_coord[KSI]*mapped_coord[KSI]);
  result[1] = -onesixteenth*(1.0 + mapped_coord[KSI])*(1.0 - 9.0*mapped_coord[KSI]*mapped_coord[KSI]);
  result[2] = -9.0*onesixteenth*(1.0 - mapped_coord[KSI]*mapped_coord[KSI])*(1.0 - 3.0*mapped_coord[KSI]);
  result[3] =  9.0*onesixteenth*(1.0 - mapped_coord[KSI]*mapped_coord[KSI])*(1.0 + 3.0*mapped_coord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void SFLineLagrangeP3::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real onesixteenth = 1.0/16.0;

  result(KSI, 0) = -onesixteenth*( 27.0*mapped_coord[KSI]*mapped_coord[KSI] - 18.0*mapped_coord[KSI] - 1.0);
  result(KSI, 1) = -onesixteenth*(-27.0*mapped_coord[KSI]*mapped_coord[KSI] - 18.0*mapped_coord[KSI] + 1.0);
  result(KSI, 2) = 9.0*onesixteenth*( 9.0*mapped_coord[KSI]*mapped_coord[KSI] - 2.0*mapped_coord[KSI] - 3.0);
  result(KSI, 3) = 9.0*onesixteenth*(-9.0*mapped_coord[KSI]*mapped_coord[KSI] - 2.0*mapped_coord[KSI] + 3.0);
}

////////////////////////////////////////////////////////////////////////////////

/// @todo Location of local nodes missing for SFLineLagrangeP3
SFLineLagrangeP3::MappedNodesT SFLineLagrangeP3::s_mapped_sf_nodes =  ( SFLineLagrangeP3::MappedNodesT() <<
  -1.,
   0.,
   0.,
   1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
