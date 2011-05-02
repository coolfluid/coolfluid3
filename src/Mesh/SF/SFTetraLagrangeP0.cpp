// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "SFTetraLagrangeP0.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SFTetraLagrangeP0, ShapeFunction, LibSF > SFTetraLagrangeP0_Builder;

////////////////////////////////////////////////////////////////////////////////

SFTetraLagrangeP0::SFTetraLagrangeP0(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void SFTetraLagrangeP0::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void SFTetraLagrangeP0::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI, 0) = 0.;
  result(ETA, 0) = 0.;
  result(ZTA, 0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix SFTetraLagrangeP0::s_mapped_sf_nodes =  ( RealMatrix(1,3) <<
   1./3.,  1./3.,  1./3.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
