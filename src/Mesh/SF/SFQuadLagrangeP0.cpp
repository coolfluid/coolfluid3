// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "SFQuadLagrangeP0.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SFQuadLagrangeP0, ShapeFunction, LibSF > SFQuadLagrangeP0_Builder;

////////////////////////////////////////////////////////////////////////////////

SFQuadLagrangeP0::SFQuadLagrangeP0(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void SFQuadLagrangeP0::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void SFQuadLagrangeP0::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI,0) = 0.;
  result(ETA,0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

SFQuadLagrangeP0::MappedNodesT SFQuadLagrangeP0::s_mapped_sf_nodes =  ( SFQuadLagrangeP0::MappedNodesT() <<
  0., 0.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
