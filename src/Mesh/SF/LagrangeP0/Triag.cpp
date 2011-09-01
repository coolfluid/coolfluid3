// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/SF/LagrangeP0/LibLagrangeP0.hpp"
#include "Mesh/SF/LagrangeP0/Triag.hpp"

namespace CF {
namespace Mesh {
namespace SF {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Triag, ShapeFunction, LibLagrangeP0 > Triag_Builder;

////////////////////////////////////////////////////////////////////////////////

Triag::Triag(const std::string& name) : ShapeFunctionT<LagrangeP0::Triag>(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = (GeoShape::Type)shape;
}

////////////////////////////////////////////////////////////////////////////////

void Triag::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Triag::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI,0) = 0.;
  result(ETA,0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix Triag::m_local_coordinates =  ( RealMatrix(Triag::nb_nodes, Triag::dimensionality) <<
   1./3., 1./3.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // SF
} // Mesh
} // CF
