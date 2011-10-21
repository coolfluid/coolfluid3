// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"
#include "common/Foreach.hpp"
#include "common/Link.hpp"
#include "common/Group.hpp"
#include "common/FindComponents.hpp"

#include "mesh/CUnifiedData.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

cf3::common::ComponentBuilder < CUnifiedData, cf3::common::Component, LibMesh > CUnifiedData_Builder;

////////////////////////////////////////////////////////////////////////////////

CUnifiedData::CUnifiedData ( const std::string& name ) : common::Component(name)
{
  m_data_indices = create_static_component_ptr<CList<Uint> >  ("data_indices");
  m_data_links   = create_static_component_ptr<common::Group>("data_links");
  m_data_indices->resize(1);
  m_data_indices->array()[0]=0;
  m_size=0;
}

////////////////////////////////////////////////////////////////////////////////

Uint CUnifiedData::unified_idx(const common::Component& component, const Uint local_idx) const
{
  std::map<common::Component const*,Uint>::const_iterator it = m_start_idx.find(&component);
  return it->second +local_idx;
}

////////////////////////////////////////////////////////////////////////////////

Uint CUnifiedData::unified_idx(const boost::tuple<common::Component::Ptr,Uint>& loc) const
{
  std::map<common::Component const*,Uint>::const_iterator it = m_start_idx.find(boost::get<0>(loc).get());
  return it->second + boost::get<1>(loc);
}

////////////////////////////////////////////////////////////////////////////////

void CUnifiedData::reset()
{
  m_start_idx.clear();
  boost_foreach(Component& data_link, m_data_links->children())
    m_data_links->remove_component(data_link);
  m_data_vector.resize(0);
  m_data_indices->resize(1);
  m_data_indices->array()[0]=0;
  m_size=0;
}

////////////////////////////////////////////////////////////////////////////////

bool CUnifiedData::contains(const common::Component& data) const
{
  return m_start_idx.find(&data) != m_start_idx.end() ;
}

////////////////////////////////////////////////////////////////////////////////

/// Get the component and local index in the component
/// given a continuous index spanning multiple components
/// @param [in] data_glb_idx continuous index covering multiple components
/// @return boost::tuple<data_type::Ptr component, Uint idx_in_component>
boost::tuple<common::Component::Ptr,Uint> CUnifiedData::location(const Uint data_glb_idx)
{
  cf3_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf3_assert(m_data_indices->array()[data_vector_idx] <= data_glb_idx );
  return boost::make_tuple(m_data_vector[data_vector_idx], data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

boost::tuple<common::Component::ConstPtr,Uint> CUnifiedData::location(const Uint data_glb_idx) const
{
  cf3_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf3_assert(m_data_indices->array()[data_vector_idx] <= data_glb_idx );
  return boost::make_tuple(m_data_vector[data_vector_idx].lock()->as_const(), data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

boost::tuple<common::Component&,Uint> CUnifiedData::location_v2(const Uint data_glb_idx)
{
  cf3_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf3_assert(m_data_indices->array()[data_vector_idx] <= data_glb_idx );
  return boost::tuple<common::Component&,Uint>(*m_data_vector[data_vector_idx].lock(), data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

boost::tuple<const common::Component&,Uint> CUnifiedData::location_v2(const Uint data_glb_idx) const
{
  cf3_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf3_assert(m_data_indices->array()[data_vector_idx] <= data_glb_idx );
  return boost::tuple<const common::Component&,Uint>(*m_data_vector[data_vector_idx].lock(), data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

/// Get the const component and local index in the component
/// given a continuous index spanning multiple components
/// @param [in] data_glb_idx continuous index covering multiple components
/// @return boost::tuple<Uint component_idx, Uint idx_in_component>
boost::tuple<Uint,Uint> CUnifiedData::location_idx(const Uint data_glb_idx) const
{
  cf3_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf3_assert(data_vector_idx<m_data_vector.size());
  return boost::make_tuple(data_vector_idx, data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

Uint CUnifiedData::location_comp_idx(const common::Component& data) const
{
  return location_idx(unified_idx(data,0)).get<0>();
}

////////////////////////////////////////////////////////////////////////////////

/// Get the total number of data spanning multiple components
/// @return the size
Uint CUnifiedData::size() const
{
  return m_size;
}

////////////////////////////////////////////////////////////////////////////////

/// non-const access to the unified data components
/// @return vector of data components
std::vector< boost::weak_ptr<common::Component> >& CUnifiedData::components()
{
  return m_data_vector;
}

////////////////////////////////////////////////////////////////////////////////

/// const access to the unified data components
/// @return vector of data components
const std::vector< boost::weak_ptr<common::Component> >& CUnifiedData::components() const
{
  return m_data_vector;
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
