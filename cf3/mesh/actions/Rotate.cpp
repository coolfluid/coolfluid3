// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StringConversion.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"

#include "mesh/actions/Rotate.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;
  
////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Rotate, MeshTransformer, mesh::actions::LibActions> Rotate_Builder;

//////////////////////////////////////////////////////////////////////////////

Rotate::Rotate( const std::string& name )
: MeshTransformer(name)
{

  properties()["brief"] = std::string("Rotate mesh");
  std::string desc;
  desc =
      "  Usage: Rotate \n\n"
      "    angle:real=theta\n"
      "\n"
      "  Optional arguments for 2D mesh:\n"
      "    - rotation centre\n"
      "        axis_point:array[real]=x,y\n"
      "\n"
      "  Optional arguments for 2D mesh:\n"
      "    - point on axis of rotation\n"
      "        axis_point:array[real]=x,y,z\n"
      "    - direction of axis of rotation\n"
      "        axis_direction:array[real]=u,v,w\n";

  properties()["description"] = desc;

  std::vector<Real> axis_direction = boost::assign::list_of(0.)(0.)(1.);
  options().add("axis_direction",axis_direction)
      .description("Direction of rotation axis, only useful for 3D rotations.")
      .mark_basic();

  std::vector<Real> axis_point     = boost::assign::list_of(0.)(0.)(0.);
  options().add("axis_point",axis_point)
      .description("Point on axis of rotation in 3D. Rotation centre in 2D")
      .mark_basic();

  options().add("angle",0.)
      .description("Rotation angle [rad]")
      .mark_basic();
}

/////////////////////////////////////////////////////////////////////////////

void Rotate::execute()
{
  // Get options
  const Real theta = options().value<Real>("angle");
  std::vector<Real> axis_dir_vec = options().value< std::vector<Real> >("axis_direction");
  std::vector<Real> axis_point_vec = options().value< std::vector<Real> >("axis_point");

  // In 2D, use 2-dimensional algorithm
  if (m_mesh->dimension() == 2u)
  {
    RealVector2 rotation_centre;
    rotation_centre << axis_point_vec[XX] , axis_point_vec[YY];
    Eigen::Matrix<Real,2,3> m = compute_rotation_matrix_2d(rotation_centre,theta);
    boost_foreach(const Handle<Dictionary>& dict, m_mesh->dictionaries())
    {
      boost_foreach(Field::Row point, dict->coordinates().array())
      {
        rotate_point_2d(m,point);
      }
    }
  }
  // In 3D, use 3-dimensional algorithm
  else if (m_mesh->dimension() == 3u)
  {
    RealVector3 point_on_axis;
    point_on_axis << axis_point_vec[XX] , axis_point_vec[YY], axis_point_vec[ZZ];
    RealVector3 axis_direction;
    axis_direction << axis_dir_vec[XX], axis_dir_vec[YY], axis_dir_vec[ZZ] ;
    Eigen::Matrix<Real,3,4> m = compute_rotation_matrix_3d(point_on_axis,axis_direction,theta);
    boost_foreach(const Handle<Dictionary>& dict, m_mesh->dictionaries())
    {
      boost_foreach(Field::Row point, dict->coordinates().array())
      {
        rotate_point_3d(m,point);
      }
    }
  }
  else
  {
    throw common::InvalidStructure(FromHere(),"Cannot rotate a mesh of dimension "+common::to_str(m_mesh->dimension()));
  }
}

//////////////////////////////////////////////////////////////////////////////

Eigen::Matrix<Real,2,3> Rotate::compute_rotation_matrix_2d(const RealVector2& rotation_centre, const Real& theta)
{
  enum { XX=0, YY=1, TT=2 };
  Eigen::Matrix<Real,2,3> m;
  
  // Rotation centre
  const Real& a = rotation_centre[XX];
  const Real& b = rotation_centre[YY];
  
  // temp values
  const Real cosT = std::cos(theta);
  const Real oneMinusCosT = 1-cosT;
  const Real sinT = std::sin(theta);
  
  // rotation matrix assembly
  m(XX,XX) =  cosT;
  m(XX,YY) = -sinT;
  m(XX,TT) =  a*oneMinusCosT + b*sinT;

  m(YY,XX) =  sinT;
  m(YY,YY) =  cosT;
  m(YY,TT) =  b*oneMinusCosT - a*sinT;
  return m;
}

//////////////////////////////////////////////////////////////////////////////

Eigen::Matrix<Real,3,4> Rotate::compute_rotation_matrix_3d(const RealVector3& point_on_axis, const RealVector3& axis_direction, const Real& theta)
{
  enum { XX=0, YY=1, ZZ=2, TT=3 };
  Eigen::Matrix<Real,3,4> m;

  // normalize axis of rotation
  const Real l = axis_direction.norm();
  const Real u = axis_direction[XX]/l;
  const Real v = axis_direction[YY]/l;
  const Real w = axis_direction[ZZ]/l;
  
  // point on axis
  const Real& a = point_on_axis[XX];
  const Real& b = point_on_axis[YY];
  const Real& c = point_on_axis[ZZ];
  
  // temp values
  const Real u2 = u*u;
  const Real v2 = v*v;
  const Real w2 = w*w;
  const Real cosT = std::cos(theta);
  const Real oneMinusCosT = 1-cosT;
  const Real sinT = std::sin(theta);
  
  // rotation matrix assembly
  m(XX,XX) = u2 + (v2 + w2) * cosT;
  m(XX,YY) = u*v * oneMinusCosT - w*sinT;
  m(XX,ZZ) = u*w * oneMinusCosT + v*sinT;
  m(XX,TT) = (a*(v2 + w2) - u*(b*v + c*w))*oneMinusCosT + (b*w - c*v)*sinT;

  m(YY,XX) = u*v * oneMinusCosT + w*sinT;
  m(YY,YY) = v2 + (u2 + w2) * cosT;
  m(YY,ZZ) = v*w * oneMinusCosT - u*sinT;
  m(YY,TT) = (b*(u2 + w2) - v*(a*u + c*w))*oneMinusCosT + (c*u - a*w)*sinT;

  m(ZZ,XX) = u*w * oneMinusCosT - v*sinT;
  m(ZZ,YY) = v*w * oneMinusCosT + u*sinT;
  m(ZZ,ZZ) = w2 + (u2 + v2) * cosT;
  m(ZZ,TT) = (c*(u2 + v2) - w*(a*u + b*v))*oneMinusCosT + (a*v - b*u)*sinT;
  return m;
}

//////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
