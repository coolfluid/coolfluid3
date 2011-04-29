// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "SFTriagLagrangeP2.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SFTriagLagrangeP2, ShapeFunction, LibSF > SFTriagLagrangeP2_Builder;

////////////////////////////////////////////////////////////////////////////////

SFTriagLagrangeP2::SFTriagLagrangeP2(const std::string& name) : ShapeFunction(name)
{
  m_dimensionality = dimensionality;
  m_nb_nodes = nb_nodes;
  m_order = order;
  m_shape = shape;
}

////////////////////////////////////////////////////////////////////////////////

void SFTriagLagrangeP2::value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real L0 = 1. - mapped_coord[KSI] - mapped_coord[ETA];
  const Real L1 = mapped_coord[KSI];
  const Real L2 = mapped_coord[ETA];

  result[0] = ( 2.*L0 - 1. ) * L0;
  result[1] = ( 2.*L1 - 1. ) * L1;
  result[2] = ( 2.*L2 - 1. ) * L2;
  result[3] = 4.*L0*L1;
  result[4] = 4.*L1*L2;
  result[5] = 4.*L2*L0;
}

////////////////////////////////////////////////////////////////////////////////

void SFTriagLagrangeP2::gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real L0 = 1. - mapped_coord[KSI] - mapped_coord[ETA];
  const Real L1 = mapped_coord[KSI];
  const Real L2 = mapped_coord[ETA];

  result(KSI, 0) = - (4.*L0-1.);
  result(ETA, 0) = - (4.*L0-1.);

  result(KSI, 1) =   (4.*L1-1.);
  result(ETA, 1) =   0.;

  result(KSI, 2) =   0.;
  result(ETA, 2) =   (4.*L2-1.);

  result(KSI, 3) =   4.*(L0-L1);
  result(ETA, 3) = - 4.*L1;

  result(KSI, 4) =   4.*L2;
  result(ETA, 4) =   4.*L1;

  result(KSI, 5) = - 4.*L2;
  result(ETA, 5) =   4.*(L0-L2);
}

////////////////////////////////////////////////////////////////////////////////

SFTriagLagrangeP2::MappedNodesT SFTriagLagrangeP2::s_mapped_sf_nodes =  ( SFTriagLagrangeP2::MappedNodesT() <<
   0.,  0.,
   1.,  0.,
   0.,  1.,
   0.5, 0.,
   0.5, 0.5,
   0.,  0.5
).finished();

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
