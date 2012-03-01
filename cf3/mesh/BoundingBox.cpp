// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/Builder.hpp"
#include "common/PropertyList.hpp"

#include "math/Consts.hpp"
#include "math/MatrixTypesConversion.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/BoundingBox.hpp"
#include "mesh/Field.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Region.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  using namespace common;
  using namespace math::Consts;

cf3::common::ComponentBuilder < BoundingBox, Component, LibMesh > BoundingBox_Builder;

//////////////////////////////////////////////////////////////////////////////

BoundingBox::BoundingBox(const std::string& name) :
  common::Component(name),
  math::BoundingBox()
{
}

////////////////////////////////////////////////////////////////////////////////

void BoundingBox::build(const Region& region)
{
  // Find dimension
  Uint dim=0;
  boost_foreach(const Entities& entities, find_components_recursively<Entities>(region))
  {
    dim = entities.element_type().dimension();
    break;
  }
  
  // find bounding box coordinates for region 1 and region 2
  min().resize(dim);
  max().resize(dim);
  min().setConstant(real_max());
  max().setConstant(real_min());
  boost_foreach(const Entities& entities, find_components_recursively<Entities>(region))
  {
    const Field& coordinates = entities.geometry_fields().coordinates();
    const Connectivity& connectivity = entities.geometry_space().connectivity();
    const Uint nb_elem = entities.size();
    for (Uint e=0; e<nb_elem; ++e)
    {
      boost_foreach(const Uint node, connectivity[e])
      {
        for (Uint d=0; d<dim; ++d)
        {
          min()[d] = std::min(min()[d],  coordinates[node][d]);
          max()[d] = std::max(max()[d],  coordinates[node][d]);
        }
      }
    }
  }
  update_properties();
}

////////////////////////////////////////////////////////////////////////////////

void BoundingBox::build(const Mesh& mesh)
{
  build(mesh.geometry_fields().coordinates());
}

////////////////////////////////////////////////////////////////////////////////

void BoundingBox::build(const Field& coordinates)
{
  // Find dimension
  Uint dim=coordinates.row_size();

  // find bounding box coordinates for region 1 and region 2
  min().resize(dim);
  max().resize(dim);
  min().setConstant(real_max());
  max().setConstant(real_min());
  boost_foreach(Field::ConstRow coords, coordinates.array())
  {
    for (Uint d=0; d<dim; ++d)
    {
      min()[d] = std::min(min()[d],  coords[d]);
      max()[d] = std::max(max()[d],  coords[d]);
    }
  }
  update_properties();
}

////////////////////////////////////////////////////////////////////////////////

void BoundingBox::update_properties()
{
  std::vector<Real> min_vec(dim());
  std::vector<Real> max_vec(dim());
  math::copy(min(),min_vec);
  math::copy(max(),max_vec);
  properties()["minimum"]=min_vec;
  properties()["maximum"]=max_vec;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
