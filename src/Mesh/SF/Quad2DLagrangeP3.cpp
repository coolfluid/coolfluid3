// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Line2DLagrangeP3.hpp"
#include "Quad2DLagrangeP3.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Quad2DLagrangeP3,
                         ElementType,
                         LibSF >
aQuad2DLagrangeP3_Builder;

////////////////////////////////////////////////////////////////////////////////

Quad2DLagrangeP3::Quad2DLagrangeP3(const std::string& name) : Quad2D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP3::compute_volume(const NodesT& coord) const
{
  return volume(coord);
}

////////////////////////////////////////////////////////////////////////////////

bool Quad2DLagrangeP3::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
  // @todo: this was copied & pasted from P1Quad code
  // make sure it works in P3 case as well
  MappedCoordsT mapped_coord;
  mapped_coordinates(CoordsT(coord), nodes, mapped_coord);
  if( (mapped_coord[KSI] >= -1.0) &&
      (mapped_coord[ETA] >= -1.0) &&
      (mapped_coord[KSI] <=  1.0) &&
      (mapped_coord[ETA] <=  1.0))
  {
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP3::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(4)(8)(12);
    connectivity.face_node_counts.assign(4, 4);
    connectivity.face_nodes = boost::assign::list_of(0)(4)(5)(1)
                                                    (1)(6)(7)(2)
                                                    (2)(8)(9)(3)
                                                    (3)(10)(11)(0);
  }
  return connectivity;
}
////////////////////////////////////////////////////////////////////////////////
// Local connectivity:
//             3---9---8---2
//             |           |
//            10  15   14  7
//             |           |
//	            11  12   13  6
//             |           |
//             0---4---5---1
// Reference domain: <-1,1> x <-1,1>

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP3::face_connectivity() const
{
  return faces();
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Quad2DLagrangeP3::face_type(const CF::Uint face) const
{
  const static Line2DLagrangeP3 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP3::shape_function_value(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  SFQuadLagrangeP3::compute_value(mappedCoord,shapeFunc);
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP3::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP3::shape_function_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  SFQuadLagrangeP3::compute_gradient(mappedCoord,result);
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP3::jacobian_determinant(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes)
{
  cf_assert(mappedCoord.size() == dimension);
  cf_assert(nodes.size() == nb_nodes);

  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP3::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP3::jacobian_adjoint(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP3::volume(const NodeMatrixT& nodes)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
