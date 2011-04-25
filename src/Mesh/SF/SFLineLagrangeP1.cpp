// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "SFLineLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SFLineLagrangeP1, ShapeFunction, LibSF > SFLineLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

SFLineLagrangeP1::SFLineLagrangeP1(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void SFLineLagrangeP1::shape_function(const MappedCoords_t& mapped_coord, ShapeFunction_t& shape_func)
{
  shape_func[0] = 0.5 * (1.0 - mapped_coord[KSI]);
  shape_func[1] = 0.5 * (1.0 + mapped_coord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void SFLineLagrangeP1::shape_function_gradient(const MappedCoords_t& mappedCoord, ShapeFunctionGradient_t& result)
{
  result(XX, 0) = -0.5;
  result(XX, 1) =  0.5;
}

////////////////////////////////////////////////////////////////////////////////

SFLineLagrangeP1::MappedNodes_t SFLineLagrangeP1::s_mapped_sf_nodes =  ( SFLineLagrangeP1::MappedNodes_t() << 
  -1.,
   1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
