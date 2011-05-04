// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "SFLineLagrangeP2.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SFLineLagrangeP2, ShapeFunction, LibSF > SFLineLagrangeP2_Builder;

////////////////////////////////////////////////////////////////////////////////

SFLineLagrangeP2::SFLineLagrangeP2(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void SFLineLagrangeP2::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 0.5 * (mapped_coord[KSI]*mapped_coord[KSI] - mapped_coord[KSI]);
  result[1] = 0.5 * (mapped_coord[KSI]*mapped_coord[KSI] + mapped_coord[KSI]);
  result[2] = (1.0 - mapped_coord[KSI]*mapped_coord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void SFLineLagrangeP2::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI, 0) = mapped_coord[KSI]-0.5;
  result(KSI, 1) = mapped_coord[KSI]+0.5;
  result(KSI, 2) = -2.0*mapped_coord[KSI];
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix SFLineLagrangeP2::s_mapped_sf_nodes =  ( RealMatrix(3,1) <<
  -1.,
   1.,
   0.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
