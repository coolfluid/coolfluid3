// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"

#include "LibSF.hpp"
#include "Line1DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Line1DLagrangeP1,
                         ElementType,
                         LibSF >
aLine1DLagrangeP1_Provider ( "Line1DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////
Line1DLagrangeP1::Line1DLagrangeP1()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Line1DLagrangeP1::getElementTypeName() const
{
  return "Line1DLagrangeP1";
}

Real Line1DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}


bool Line1DLagrangeP1::is_coord_in_element( const RealVector& coord, const NodesT& nodes) const
{
  MappedCoordsT mapped_coord;
  mapped_coordinates(CoordsT(coord), nodes, mapped_coord);
  if( (mapped_coord[KSI] >= -0.5) &&
      (mapped_coord[KSI] <= 0.5) )
  {
    return true;
  }
  else
  {
    return false;
  }
}

const CF::Mesh::ElementType::FaceConnectivity& Line1DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  return connectivity;
}

const CF::Mesh::ElementType& Line1DLagrangeP1::face_type(const CF::Uint face) const
{
  // TODO: Add a Point1DLagrangeP1 type to complete this
  throw Common::NotImplemented(FromHere(), "Line1DLagrangeP1::face_type requires a point type");
}


} // SF
} // Mesh
} // CF
