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
#include "mesh/LagrangeP1/Tetra3D.hpp"
#include "mesh/LagrangeP1/Triag3D.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ElementTypeT<Tetra3D>, ElementType , LibLagrangeP1 >
   Tetra3D_Builder(LibLagrangeP1::library_namespace()+"."+Tetra3D::type_name());

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType::FaceConnectivity& Tetra3D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0)(3)(6)(9);
    connectivity.stride.assign(nb_faces, 3);
    connectivity.nodes = boost::assign::list_of(0)(2)(1)
                                               (0)(1)(3)
                                               (1)(2)(3)
                                               (0)(3)(2);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const cf3::mesh::ElementType& Tetra3D::face_type(const cf3::Uint face)
{
  static const boost::shared_ptr< ElementType > facetype( common::allocate_component<ElementTypeT<LagrangeP1::Triag3D> >(LagrangeP1::Triag3D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Tetra3D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = 0.25*(nodes(0,XX)+nodes(1,XX)+nodes(2,XX)+nodes(3,XX));
  centroid[YY] = 0.25*(nodes(0,YY)+nodes(1,YY)+nodes(2,YY)+nodes(3,YY));
  centroid[ZZ] = 0.25*(nodes(0,ZZ)+nodes(1,ZZ)+nodes(2,ZZ)+nodes(3,ZZ));
}

////////////////////////////////////////////////////////////////////////////////

bool Tetra3D::is_coord_in_element(const CoordsT& coord, const NodesT& nodes)
{
  MappedCoordsT mapped_coord = mapped_coordinate(coord,nodes);
  const Real tolerance = 10*math::Consts::eps();
  if((mapped_coord[KSI] >= -tolerance) &&
     (mapped_coord[ETA] >= -tolerance) &&
     (mapped_coord[ZTA] >= -tolerance) &&
     (mapped_coord.sum() <= 1.+tolerance))
  {
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

void Tetra3D::compute_mapped_coordinate(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mapped_coord)
{
  RealMatrix3 M;
  M.col(0) = nodes.row(1) - nodes.row(0);
  M.col(1) = nodes.row(2) - nodes.row(0);
  M.col(2) = nodes.row(3) - nodes.row(0);

  mapped_coord.noalias() = M.inverse() * (coord - nodes.row(0).transpose());
}

////////////////////////////////////////////////////////////////////////////////

Tetra3D::MappedCoordsT Tetra3D::mapped_coordinate(const CoordsT& coord, const NodesT& nodes)
{
  MappedCoordsT result;
  compute_mapped_coordinate(coord,nodes,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Tetra3D::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  const Real x0 = nodes(0, XX);
  const Real y0 = nodes(0, YY);
  const Real z0 = nodes(0, ZZ);

  const Real x1 = nodes(1, XX);
  const Real y1 = nodes(1, YY);
  const Real z1 = nodes(1, ZZ);

  const Real x2 = nodes(2, XX);
  const Real y2 = nodes(2, YY);
  const Real z2 = nodes(2, ZZ);

  const Real x3 = nodes(3, XX);
  const Real y3 = nodes(3, YY);
  const Real z3 = nodes(3, ZZ);

  return
      x2*y1*z0 - x3*y1*z0 - x1*y2*z0 + x3*y2*z0 + x1*y3*z0 -
      x2*y3*z0 - x2*y0*z1 + x3*y0*z1 + x0*y2*z1 - x3*y2*z1 -
      x0*y3*z1 + x2*y3*z1 + x1*y0*z2 - x3*y0*z2 - x0*y1*z2 +
      x3*y1*z2 + x0*y3*z2 - x1*y3*z2 - x1*y0*z3 + x2*y0*z3 +
      x0*y1*z3 - x2*y1*z3 - x0*y2*z3 + x1*y2*z3;
}

////////////////////////////////////////////////////////////////////////////////

void Tetra3D::compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  const Real x0 = nodes(0, XX);
  const Real y0 = nodes(0, YY);
  const Real z0 = nodes(0, ZZ);

  const Real x1 = nodes(1, XX);
  const Real y1 = nodes(1, YY);
  const Real z1 = nodes(1, ZZ);

  const Real x2 = nodes(2, XX);
  const Real y2 = nodes(2, YY);
  const Real z2 = nodes(2, ZZ);

  const Real x3 = nodes(3, XX);
  const Real y3 = nodes(3, YY);
  const Real z3 = nodes(3, ZZ);

  const Real dxdksi = -x0 + x1;
  const Real dydksi = -y0 + y1;
  const Real dzdksi = -z0 + z1;

  const Real dxdeta = -x0 + x2;
  const Real dydeta = -y0 + y2;
  const Real dzdeta = -z0 + z2;

  const Real dxdzta = -x0 + x3;
  const Real dydzta = -y0 + y3;
  const Real dzdzta = -z0 + z3;

  // Derivatives of shape functions are constant
  // hence Jacobians are independent of the mappedCoord
  result(KSI,XX) = dxdksi;
  result(KSI,YY) = dydksi;
  result(KSI,ZZ) = dzdksi;

  result(ETA,XX) = dxdeta;
  result(ETA,YY) = dydeta;
  result(ETA,ZZ) = dzdeta;

  result(ZTA,XX) = dxdzta;
  result(ZTA,YY) = dydzta;
  result(ZTA,ZZ) = dzdzta;
}

////////////////////////////////////////////////////////////////////////////////

Tetra3D::JacobianT Tetra3D::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

void Tetra3D::compute_jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  JacobianT J = jacobian(mapped_coord, nodes);
  result(0, 0) =  (J(1, 1)*J(2, 2) - J(1, 2)*J(2, 1));
  result(0, 1) = -(J(0, 1)*J(2, 2) - J(0, 2)*J(2, 1));
  result(0, 2) =  (J(0, 1)*J(1, 2) - J(1, 1)*J(0, 2));
  result(1, 0) = -(J(1, 0)*J(2, 2) - J(1, 2)*J(2, 0));
  result(1, 1) =  (J(0, 0)*J(2, 2) - J(0, 2)*J(2, 0));
  result(1, 2) = -(J(0, 0)*J(1, 2) - J(0, 2)*J(1, 0));
  result(2, 0) =  (J(1, 0)*J(2, 1) - J(1, 1)*J(2, 0));
  result(2, 1) = -(J(0, 0)*J(2, 1) - J(0, 1)*J(2, 0));
  result(2, 2) =  (J(0, 0)*J(1, 1) - J(0, 1)*J(1, 0));
}

////////////////////////////////////////////////////////////////////////////////

Real Tetra3D::volume(const NodesT& nodes)
{
  return jacobian_determinant(MappedCoordsT::Zero(),nodes) / 6.;
}

////////////////////////////////////////////////////////////////////////////////

Real Tetra3D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
