// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Line2DLagrangeP2.hpp"
#include "Quad2DLagrangeP2.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Quad2DLagrangeP2,
                         ElementType,
                         LibSF >
aQuad2DLagrangeP2_Builder;

////////////////////////////////////////////////////////////////////////////////

Quad2DLagrangeP2::Quad2DLagrangeP2(const std::string& name) : Quad2D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP2::compute_volume(const NodesT& coord) const
{
  return volume(coord);
}

////////////////////////////////////////////////////////////////////////////////

bool Quad2DLagrangeP2::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
  // @todo: this was copied & pasted from P1Quad code
  // make sure it works in P2 case as well
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

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP2::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(3)(6)(9);
    connectivity.face_node_counts.assign(4, 3);
    connectivity.face_nodes = boost::assign::list_of(0)(1)(4)
                                                    (1)(2)(5)
                                                    (2)(3)(6)
                                                    (3)(0)(7);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP2::face_connectivity() const
{
  return faces();
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Quad2DLagrangeP2::face_type(const CF::Uint face) const
{
  const static Line2DLagrangeP2 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP2::shape_function_value(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  SFQuadLagrangeP2::compute_value(mappedCoord,shapeFunc);
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP2::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP2::shape_function_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  SFQuadLagrangeP2::compute_gradient(mappedCoord,result);
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP2::jacobian_determinant(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes)
{
  cf_assert(mappedCoord.size() == dimension);
  cf_assert(nodes.size() == nb_nodes);

  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP2::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP2::jacobian_adjoint(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result) 
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP2::volume(const NodeMatrixT& nodes) 
{

  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);
  const Real x3 = nodes(3, XX);
  const Real x4 = nodes(4, XX);
  const Real x5 = nodes(5, XX);
  const Real x6 = nodes(6, XX);
  const Real x7 = nodes(7, XX);
  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);
  const Real y3 = nodes(3, YY);
  const Real y4 = nodes(4, YY);
  const Real y5 = nodes(5, YY);
  const Real y6 = nodes(6, YY);
  const Real y7 = nodes(7, YY);

  return (4*((x7 - x4)*y0 + (x4 - x5)*y1 + (x5 - x6)*y2 + (x6 - x7)*y3) +
          x1*(y0 - y2 - 4*y4 + 4*y5) + x2*(y1 - y3 - 4*y5 + 4*y6) +
          x0*(y3 - y1 + 4*y4 - 4*y7) + x3*(y2 - y0 - 4*y6 + 4*y7)
         )/6.;
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
