// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

ElementType::ElementType( const std::string& name ) : Common::Component(name)
{

}

ElementType::~ElementType()
{
}

////////////////////////////////////////////////////////////////////////////////

void ElementType::compute_normal(const NodesT& coord, RealVector& normal) const
{
  throw Common::NotImplemented(FromHere(),"compute_normal not implemented for "+derived_type_name());
}

////////////////////////////////////////////////////////////////////////////////

void ElementType::compute_centroid(const NodesT& coord , RealVector& centroid) const
{
  throw Common::NotImplemented(FromHere(),"compute_centroid not implemented for "+derived_type_name());
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& ElementType::shape_function() const
{
  throw Common::NotImplemented(FromHere(),"shape_function() not implemented for "+derived_type_name()+"\nThis element type is not compatible yet. It should have a dedicated separate shape function.\nCheck src/Mesh/SF/Line.hpp and the LineLagrange element types for examples.");
  static const ShapeFunction f;
  return f;
}

////////////////////////////////////////////////////////////////////////////////

std::string ElementType::builder_name() const
{
  return derived_type_name();
}

////////////////////////////////////////////////////////////////////////////////

/// Return the face connectivity information
const ElementType::FaceConnectivity& ElementType::face_connectivity() const
{
  throw Common::NotImplemented(FromHere(),"face_connectivity() not implemented for "+derived_type_name());
  static ElementType::FaceConnectivity connectivity;
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const ElementType& ElementType::face_type(const Uint face) const
{
  throw Common::NotImplemented(FromHere(),"face_type(const Uint face) not implemented for "+derived_type_name());
  static const ElementType f_type;
  return f_type;
}

////////////////////////////////////////////////////////////////////////////////

bool ElementType::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
  throw Common::NotImplemented(FromHere(),"is_coord_in_element(const RealVector& coord, const NodesT& nodes) not implemented for "+derived_type_name());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

Real ElementType::compute_volume(const NodesT& coord) const
{
  throw Common::NotImplemented(FromHere(),"compute_volume not implemented for "+derived_type_name());
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real ElementType::compute_area(const NodesT& coord) const
{
  throw Common::NotImplemented(FromHere(),"compute_area not implemented for "+derived_type_name());
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real ElementType::jacobian_determinant(const RealVector& mapped_coord, const RealMatrix& nodes) const
{
  throw Common::NotImplemented(FromHere(),"jacobian_determinant not implemented for "+derived_type_name());
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix ElementType::jacobian(const RealVector& mapped_coord, const RealMatrix& nodes) const
{
  throw Common::NotImplemented(FromHere(),"jacobian not implemented for "+derived_type_name());
  return RealMatrix(0,0);
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
