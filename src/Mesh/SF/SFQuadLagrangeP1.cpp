// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "SFQuadLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SFQuadLagrangeP1, ShapeFunction, LibSF > SFQuadLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

SFQuadLagrangeP1::SFQuadLagrangeP1(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void SFQuadLagrangeP1::value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real ksi  = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  result[0] = 0.25 * (1.0 - ksi) * (1.0 - eta);
  result[1] = 0.25 * (1.0 + ksi) * (1.0 - eta);
  result[2] = 0.25 * (1.0 + ksi) * (1.0 + eta);
  result[3] = 0.25 * (1.0 - ksi) * (1.0 + eta);
}

////////////////////////////////////////////////////////////////////////////////

void SFQuadLagrangeP1::gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real ksi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  result(KSI, 0) = 0.25 * (-1. + eta);
  result(ETA, 0) = 0.25 * (-1. + ksi);
  result(KSI, 1) = 0.25 * ( 1. - eta);
  result(ETA, 1) = 0.25 * (-1. - ksi);
  result(KSI, 2) = 0.25 * ( 1. + eta);
  result(ETA, 2) = 0.25 * ( 1. + ksi);
  result(KSI, 3) = 0.25 * (-1. - eta);
  result(ETA, 3) = 0.25 * ( 1. - ksi);
}

////////////////////////////////////////////////////////////////////////////////

SFQuadLagrangeP1::MappedNodesT SFQuadLagrangeP1::s_mapped_sf_nodes =  ( SFQuadLagrangeP1::MappedNodesT() <<
  -1., -1.,
   1., -1.,
   1.,  1.,
  -1.,  1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
