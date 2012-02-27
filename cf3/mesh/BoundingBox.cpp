// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Foreach.hpp"
#include "common/Builder.hpp"

#include "common/PE/Comm.hpp"

#include "math/Consts.hpp"

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
  common::Component(name)
{
}

//////////////////////////////////////////////////////////////////////////////

void BoundingBox::define(const RealVector& min, const RealVector& max)
{
  m_bounding_min = min;
  m_bounding_max = max;
}

//////////////////////////////////////////////////////////////////////////////

void BoundingBox::define(const std::vector<Real>& min, const std::vector<Real>& max)
{
  m_bounding_min.resize(min.size());
  m_bounding_max.resize(max.size());
  math::copy(min,m_bounding_min);
  math::copy(max,m_bounding_max);
}

//////////////////////////////////////////////////////////////////////////////

void BoundingBox::make_global()
{
  // Find global minimum and global maximum
  std::vector<Real> bounding_min(dim());
  std::vector<Real> bounding_max(dim());
  math::copy(min(),bounding_min);
  math::copy(max(),bounding_max);
  PE::Comm::instance().all_reduce(PE::min(),bounding_min,bounding_min);
  PE::Comm::instance().all_reduce(PE::max(),bounding_max,bounding_max);

  // Copy global minimum and global maximum in bounding box
  math::copy(bounding_min,min());
  math::copy(bounding_max,max());
}

//////////////////////////////////////////////////////////////////////////////

bool BoundingBox::contains(const RealVector& coordinate) const
{
  bool inside=true;
  for (Uint d=0; d<coordinate.size(); ++d)
  {
    inside = inside && (coordinate[d]>=m_bounding_min[d] && coordinate[d]<=m_bounding_max[d]);
  }
  return inside;
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
  m_bounding_min.resize(dim);
  m_bounding_max.resize(dim);
  m_bounding_min.setConstant(real_max());
  m_bounding_max.setConstant(real_min());
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
          m_bounding_min[d] = std::min(m_bounding_min[d],  coordinates[node][d]);
          m_bounding_max[d] = std::max(m_bounding_max[d],  coordinates[node][d]);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void BoundingBox::build(const Mesh& mesh)
{
  build(mesh.topology());
}

////////////////////////////////////////////////////////////////////////////////

void BoundingBox::build(const Field& coordinates)
{
  // Find dimension
  Uint dim=coordinates.row_size();

  // find bounding box coordinates for region 1 and region 2
  m_bounding_min.resize(dim);
  m_bounding_max.resize(dim);
  m_bounding_min.setConstant(real_max());
  m_bounding_max.setConstant(real_min());
  boost_foreach(Field::ConstRow coords, coordinates.array())
  {
    for (Uint d=0; d<dim; ++d)
    {
      m_bounding_min[d] = std::min(m_bounding_min[d],  coords[d]);
      m_bounding_max[d] = std::max(m_bounding_max[d],  coords[d]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
