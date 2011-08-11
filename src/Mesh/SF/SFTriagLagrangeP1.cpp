// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "SFTriagLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SFTriagLagrangeP1, ShapeFunction, LibSF > SFTriagLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

SFTriagLagrangeP1::SFTriagLagrangeP1(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void SFTriagLagrangeP1::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 1. - mapped_coord[KSI] - mapped_coord[ETA];
  result[1] = mapped_coord[KSI];
  result[2] = mapped_coord[ETA];
}

////////////////////////////////////////////////////////////////////////////////

void SFTriagLagrangeP1::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI, 0) = -1.;
  result(ETA, 0) = -1.;
  result(KSI, 1) =  1.;
  result(ETA, 1) =  0.;
  result(KSI, 2) =  0.;
  result(ETA, 2) =  1.;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix SFTriagLagrangeP1::s_mapped_sf_nodes =  ( RealMatrix(3,2) <<
   0.,  0.,
   1.,  0.,
   0.,  1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
