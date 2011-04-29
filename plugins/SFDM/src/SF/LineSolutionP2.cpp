// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "LineSolutionP2.hpp"

namespace CF {
namespace SFDM {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < LineSolutionP2, Mesh::ShapeFunction, LibSF > LineSolutionP2_Builder;

////////////////////////////////////////////////////////////////////////////////

LineSolutionP2::LineSolutionP2(const std::string& name) : Mesh::ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void LineSolutionP2::value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real ksi2 = mapped_coord[KSI]*mapped_coord[KSI];
  result[0] = 0.5 * (ksi2 - mapped_coord[KSI]);
  result[1] = 0.5 * (ksi2 + mapped_coord[KSI]);
  result[2] = (1. - ksi2);
}

////////////////////////////////////////////////////////////////////////////////

void LineSolutionP2::gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI, 0) = mapped_coord[KSI]-0.5;
  result(KSI, 1) = mapped_coord[KSI]+0.5;
  result(KSI, 2) = -2.*mapped_coord[KSI];
}

////////////////////////////////////////////////////////////////////////////////

LineSolutionP2::MappedNodesT LineSolutionP2::s_mapped_sf_nodes =  ( LineSolutionP2::MappedNodesT() <<
  -1.,
   0.,
   1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF
