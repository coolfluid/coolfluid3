// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Line2DLagrangeP2.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Line2DLagrangeP2,
                         ElementType,
                         LibSF >
aLine2DLagrangeP2_Builder;

////////////////////////////////////////////////////////////////////////////////

Line2DLagrangeP2::Line2DLagrangeP2(const std::string& name) : Line2D(name)
{
   

  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Line2DLagrangeP2::element_type_name() const
{
  return LibSF::library_namespace() + "." + type_name();
}

Real Line2DLagrangeP2::compute_volume(const NodesT& coord) const
{
  return 0;
}

bool Line2DLagrangeP2::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return false;
}

const CF::Mesh::ElementType::FaceConnectivity& Line2DLagrangeP2::face_connectivity() const
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0);
    connectivity.face_node_counts.assign(1, 3); //1 row, 3 columns - indexes 0,1,2 of the nodes of the line
    connectivity.face_nodes = boost::assign::list_of(0)(1)(2);
  }
  return connectivity;
}

const CF::Mesh::ElementType& Line2DLagrangeP2::face_type(const CF::Uint face) const
{
  static const Line2DLagrangeP2 facetype;
  return facetype;
}



} // SF
} // Mesh
} // CF
