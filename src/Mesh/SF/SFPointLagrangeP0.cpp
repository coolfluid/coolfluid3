// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "SFPointLagrangeP0.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SFPointLagrangeP0, ShapeFunction, LibSF > SFPointLagrangeP0_Builder;

////////////////////////////////////////////////////////////////////////////////

SFPointLagrangeP0::SFPointLagrangeP0(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void SFPointLagrangeP0::compute_value(const MappedCoordsT& mapped_coord, ValueT& shape_func)
{
  shape_func[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void SFPointLagrangeP0::compute_gradient(const MappedCoordsT& mappedCoord, GradientT& result)
{
  result(KSI, 0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

SFPointLagrangeP0::MappedNodesT SFPointLagrangeP0::s_mapped_sf_nodes =  ( SFPointLagrangeP0::MappedNodesT() <<
  0.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
