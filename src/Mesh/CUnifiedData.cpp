// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CUnifiedData.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

CF::Common::ComponentBuilder < CUnifiedData, CF::Common::Component, LibMesh > CUnifiedData_Builder;

////////////////////////////////////////////////////////////////////////////////

CUnifiedData::CUnifiedData ( const std::string& name ) : Common::Component(name)
{
  m_data_indices = create_static_component_ptr<CList<Uint> >  ("data_indices");
  m_data_links   = create_static_component_ptr<Common::CGroup>("data_links");
  m_data_indices->resize(1);
  m_data_indices->array()[0]=0;
  m_size=0;
}

////////////////////////////////////////////////////////////////////////////////

Uint CUnifiedData::unified_idx(const Common::Component& component, const Uint local_idx) const
{
  std::map<Component::Ptr,Uint>::const_iterator it = m_start_idx.find(component.as_non_const());
  return it->second +local_idx;
}

////////////////////////////////////////////////////////////////////////////////

Uint CUnifiedData::unified_idx(const boost::tuple<Common::Component::Ptr,Uint>& loc) const
{
  std::map<Component::Ptr,Uint>::const_iterator it = m_start_idx.find(boost::get<0>(loc));
  return it->second + boost::get<1>(loc);
}

////////////////////////////////////////////////////////////////////////////////

void CUnifiedData::reset()
{
  m_data_vector.resize(0);
  m_data_indices->resize(0);
  boost_foreach(Common::Component& link, find_components(*m_data_links))
    m_data_links->remove_component(link.name());
  m_size = 0;
  m_start_idx.clear();
}

////////////////////////////////////////////////////////////////////////////////

bool CUnifiedData::contains(const Common::Component& data) const
{
  return m_start_idx.find(data.as_non_const()) != m_start_idx.end() ;
}

////////////////////////////////////////////////////////////////////////////////

/// Get the component and local index in the component
/// given a continuous index spanning multiple components
/// @param [in] data_glb_idx continuous index covering multiple components
/// @return boost::tuple<data_type::Ptr component, Uint idx_in_component>
boost::tuple<Common::Component::Ptr,Uint> CUnifiedData::location(const Uint data_glb_idx)
{
  cf_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf_assert(m_data_indices->array()[data_vector_idx] <= data_glb_idx );
  return boost::make_tuple(m_data_vector[data_vector_idx], data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

boost::tuple<Common::Component::ConstPtr,Uint> CUnifiedData::location(const Uint data_glb_idx) const
{
  cf_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf_assert(m_data_indices->array()[data_vector_idx] <= data_glb_idx );
  return boost::make_tuple(m_data_vector[data_vector_idx]->as_const(), data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

boost::tuple<Common::Component&,Uint> CUnifiedData::location_v2(const Uint data_glb_idx)
{
  cf_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf_assert(m_data_indices->array()[data_vector_idx] <= data_glb_idx );
  return boost::tuple<Common::Component&,Uint>(*m_data_vector[data_vector_idx], data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

boost::tuple<const Common::Component&,Uint> CUnifiedData::location_v2(const Uint data_glb_idx) const
{
  cf_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf_assert(m_data_indices->array()[data_vector_idx] <= data_glb_idx );
  return boost::tuple<const Common::Component&,Uint>(*m_data_vector[data_vector_idx], data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

/// Get the const component and local index in the component
/// given a continuous index spanning multiple components
/// @param [in] data_glb_idx continuous index covering multiple components
/// @return boost::tuple<Uint component_idx, Uint idx_in_component>
boost::tuple<Uint,Uint> CUnifiedData::location_idx(const Uint data_glb_idx) const
{
  cf_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf_assert(data_vector_idx<m_data_vector.size());
  return boost::make_tuple(data_vector_idx, data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

Uint CUnifiedData::location_comp_idx(const Common::Component& data) const
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
std::vector<Common::Component::Ptr>& CUnifiedData::components()
{
  return m_data_vector;
}

////////////////////////////////////////////////////////////////////////////////

/// const access to the unified data components
/// @return vector of data components
const std::vector<Common::Component::Ptr>& CUnifiedData::components() const
{
  return m_data_vector;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
