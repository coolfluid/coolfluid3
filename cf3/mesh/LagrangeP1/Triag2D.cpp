// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Builder.hpp"

#include "math/Consts.hpp"

#include "mesh/ElementTypeT.hpp"

#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Triag2D.hpp"
#include "mesh/LagrangeP1/Line2D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

struct ElementTypeTriag2D : ElementTypeT<Triag2D>
{
  ElementTypeTriag2D( const std::string& name = type_name() ) : ElementTypeT<Triag2D>(name) {}
  
  virtual void compute_jacobian_adjoint ( const RealVector& mapped_coord, const RealMatrix& nodes, RealMatrix& jacobian_adjoint ) const
  {
    Triag2D::compute_jacobian_adjoint(mapped_coord, nodes, jacobian_adjoint);
  }
  
  static std::string type_name() { return  ElementTypeT<Triag2D>::type_name(); }
};

common::ComponentBuilder < ElementTypeTriag2D, ElementType , LibLagrangeP1 >
   Triag2D_Builder(LibLagrangeP1::library_namespace()+"."+Triag2D::type_name());

////////////////////////////////////////////////////////////////////////////////


/// Stand-alone helper function for reuse in volume() and jacobian_determinant()
Real jacobian_determinant_helper(const Triag2D::NodesT& nodes)
{
  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);
  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);
  return (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Triag2D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0)(2)(4);
    connectivity.stride.assign(nb_faces, 2);
    connectivity.nodes = boost::assign::list_of(0)(1)
                                               (1)(2)
                                               (2)(0);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Triag2D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<LagrangeP1::Line2D> >(LagrangeP1::Line2D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Triag2D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = nodes(0,XX)+nodes(1,XX)+nodes(2,XX);
  centroid[YY] = nodes(0,YY)+nodes(1,YY)+nodes(2,YY);
  centroid /= 3.;
}

////////////////////////////////////////////////////////////////////////////////

bool Triag2D::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  MappedCoordsT mapped_coord = mapped_coordinate(coord,nodes);
  if( (mapped_coord[KSI] >= -math::Consts::eps()) &&
      (mapped_coord[ETA] >= -math::Consts::eps()) &&
      (mapped_coord.sum() <= 1.))
  {
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

void Triag2D::compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord)
{
  const Real invDet = 1. / jacobian_determinant_helper(nodes);

  mapped_coord[KSI] = invDet * ((nodes(2, YY) - nodes(0, YY))*coord[XX] + (nodes(0, XX) - nodes(2, XX))*coord[YY] - nodes(0, XX)*nodes(2, YY) + nodes(2, XX)*nodes(0, YY));
  mapped_coord[ETA] = invDet * ((nodes(0, YY) - nodes(1, YY))*coord[XX] + (nodes(1, XX) - nodes(0, XX))*coord[YY] + nodes(0, XX)*nodes(1, YY) - nodes(1, XX)*nodes(0, YY));
}

////////////////////////////////////////////////////////////////////////////////

Triag2D::MappedCoordsT Triag2D::mapped_coordinate(const CoordsT& coord, const NodesT& nodes)
{
  MappedCoordsT result;
  compute_mapped_coordinate(coord,nodes,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Triag2D::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  return jacobian_determinant_helper(nodes);
}

////////////////////////////////////////////////////////////////////////////////

void Triag2D::compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  result(KSI,XX) = nodes(1, XX) - nodes(0, XX);
  result(KSI,YY) = nodes(1, YY) - nodes(0, YY);
  result(ETA,XX) = nodes(2, XX) - nodes(0, XX);
  result(ETA,YY) = nodes(2, YY) - nodes(0, YY);
}

////////////////////////////////////////////////////////////////////////////////

Triag2D::JacobianT Triag2D::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Triag2D::volume(const NodesT& nodes)
{
  return 0.5 * jacobian_determinant_helper(nodes);
}

////////////////////////////////////////////////////////////////////////////////

Real Triag2D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
