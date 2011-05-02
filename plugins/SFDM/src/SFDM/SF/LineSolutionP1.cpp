// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/SF/LineSolutionP1.hpp"

namespace CF {
namespace SFDM {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < LineSolutionP1, Mesh::ShapeFunction, LibSF > LineSolutionP1_Builder;

////////////////////////////////////////////////////////////////////////////////

LineSolutionP1::LineSolutionP1(const std::string& name) : Mesh::ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void LineSolutionP1::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 0.5 * (1.0 - mapped_coord[KSI]);
  result[1] = 0.5 * (1.0 + mapped_coord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void LineSolutionP1::compute_gradient(const MappedCoordsT& mappedCoord, GradientT& result)
{
  result(KSI, 0) = -0.5;
  result(KSI, 1) =  0.5;
}

////////////////////////////////////////////////////////////////////////////////

LineSolutionP1::MappedNodesT LineSolutionP1::s_mapped_sf_nodes =  ( LineSolutionP1::MappedNodesT() <<
  -1.,
   1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF
