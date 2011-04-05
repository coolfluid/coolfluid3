// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CUnifiedData_hpp
#define CF_Mesh_CUnifiedData_hpp

#include <boost/tuple/tuple.hpp>

#include "Common/Foreach.hpp"
#include "Common/Component.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"
#include "Common/FindComponents.hpp"
#include "Common/StringConversion.hpp"

#include "Mesh/CList.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// This class allows to access data spread over multiple components
/// with a continuous index
/// @pre the data components must be of the same type and must have
///      a member function "Uint size() const" defined.
template <typename DATA>
class Mesh_API CUnifiedData : public Common::Component
{
public:

  typedef boost::shared_ptr<CUnifiedData> Ptr;
  typedef boost::shared_ptr<CUnifiedData const> ConstPtr;

  typedef DATA data_type;

  typedef boost::tuple<boost::shared_ptr<data_type const>, Uint> const_data_location_type;
  typedef boost::tuple<boost::shared_ptr<data_type>, Uint>       data_location_type;

  /// Contructor
  /// @param name of the component
  CUnifiedData ( const std::string& name );

  /// Virtual destructor
  virtual ~CUnifiedData() {}

  /// Get the class name
  static std::string type_name () { return "CUnifiedData<"+data_type::type_name()+">"; }
  
/*  void add(std::vector<boost::shared_ptr<DATA> >& range )
  {
    CList<Uint>::Buffer data_start_indices = m_data_indices->create_buffer();
    boost_foreach(Component::Ptr data_val, range)
    {
      typename CUnifiedData<DATA>::data_type::Ptr linked = data_val->follow()->as_ptr<typename CUnifiedData<DATA>::data_type>(); // in case it is a link
      m_data_links->create_component<Common::CLink>("data_component_"+ to_str(m_data_vector.size()))->link_to(linked);
      m_data_vector.push_back(linked);
      m_size += linked->size();
      data_start_indices.add_row(m_size);
    }

    data_start_indices.flush();

    cf_assert(m_data_vector.size()==m_data_indices->size()-1);
    
  }*/
  
  /// set the data given a range of components obtained for instance
  /// using the find_components_recursively<data_type>() function
  /// @param [in] range  The range of data components to be unified
  void add(DATA& data);

  void add(Common::CLink& data);
  

  /// Get the component and local index in the component
  /// given a continuous index spanning multiple components
  /// @param [in] data_glb_idx continuous index covering multiple components
  /// @return boost::tuple<data_type::Ptr component, Uint idx_in_component>
  data_location_type location(const Uint data_glb_idx);

  /// Get the const component and local index in the component
  /// given a continuous index spanning multiple components
  /// @param [in] data_glb_idx continuous index covering multiple components
  /// @return boost::tuple<data_type::ConstPtr component, Uint idx_in_component>
  const_data_location_type location(const Uint data_glb_idx) const;

  boost::tuple<Uint,Uint> location_idx(const Uint data_glb_idx) const;

  /// Get the total number of data spanning multiple components
  /// @return the size
  Uint size() const;

  /// non-const access to the unified data components
  /// @return vector of data components
  std::vector<boost::shared_ptr<data_type> >& data_components();

  /// const access to the unified data components
  /// @return vector of data components
  const std::vector<boost::shared_ptr<data_type> >& data_components() const;

  Common::CGroup& data_links() { return *m_data_links; }

  Uint unified_idx(data_type& component, const Uint local_idx)
  {
    return m_start_idx[component.as_ptr<data_type>()]+local_idx;
  }

  Uint unified_idx(data_location_type& loc)
  {
    return m_start_idx[boost::get<0>(loc)] + boost::get<1>(loc);
  }
  
  void reset()
  {
    m_data_vector.resize(0);
    m_data_indices->resize(0);
    boost_foreach(Component& link, find_components(*m_data_links))
      m_data_links->remove_component(link.name());
    m_size = 0;
    m_start_idx.clear();
  }
  
  bool contains(data_type& data) const
  {
    return m_start_idx.find(data.as_ptr<data_type>()) != m_start_idx.end() ;
  }

private: // data

  /// vector of components to view as continuous
  std::vector<boost::shared_ptr<data_type> > m_data_vector;

  /// start index for each component in the continous view
  boost::shared_ptr<CList<Uint> > m_data_indices;

  /// group with links to data components (links to elements of m_data_vector)
  Common::CGroup::Ptr m_data_links;

  /// total number of indices spanning all components
  Uint m_size;
  
  /// map component to start index
  std::map<boost::shared_ptr<data_type>,Uint> m_start_idx;

}; // CUnifiedData

////////////////////////////////////////////////////////////////////////////////

template <typename DATA>
inline CUnifiedData<DATA>::CUnifiedData ( const std::string& name ) : Component(name)
{
  regist_typeinfo(this);
  m_data_indices = create_static_component<CList<Uint> >  ("data_indices");
  m_data_links   = create_static_component<Common::CGroup>("data_links");
  m_data_indices->resize(1);
  m_data_indices->array()[0]=0;
  m_size=0;
}

////////////////////////////////////////////////////////////////////////////////

/// set the data given a range of components obtained for instance
/// using the find_components_recursively<data_type>() function
/// @param [in] range  The range of data components to be unified
template <typename DATA>
inline void CUnifiedData<DATA>::add(DATA& data)
{
  boost::shared_ptr<DATA> actual_data = data.as_ptr<DATA>();
  
  // if it is not added yet, add
  if (m_start_idx.find(actual_data) == m_start_idx.end())
  {
    m_start_idx[actual_data] = m_size;  

    m_data_links->create_component<Common::CLink>("data_component_"+Common::to_str(m_data_vector.size()))->link_to(*actual_data);

    m_start_idx[actual_data] = m_size;  
    m_data_vector.push_back(actual_data);
    m_size += actual_data->size();

    CList<Uint>::Buffer data_start_indices = m_data_indices->create_buffer();
    data_start_indices.add_row(m_size);
    data_start_indices.flush();
  }
  
  cf_assert(m_data_vector.size()==m_data_indices->size()-1);
  cf_assert(m_start_idx.size() == m_data_vector.size());
}

////////////////////////////////////////////////////////////////////////////////

/// set the data given a range of components obtained for instance
/// using the find_components_recursively<data_type>() function
/// @param [in] range  The range of data components to be unified
template <typename DATA>
inline void CUnifiedData<DATA>::add(Common::CLink& link)
{
  add(link.follow()->as_type<DATA>());
}

////////////////////////////////////////////////////////////////////////////////

/// Get the component and local index in the component
/// given a continuous index spanning multiple components
/// @param [in] data_glb_idx continuous index covering multiple components
/// @return boost::tuple<data_type::Ptr component, Uint idx_in_component>
template <typename DATA>
inline typename CUnifiedData<DATA>::data_location_type CUnifiedData<DATA>::location(const Uint data_glb_idx)
{
  cf_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf_assert(m_data_indices->array()[data_vector_idx] <= data_glb_idx );
  return boost::make_tuple(m_data_vector[data_vector_idx], data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

/// Get the const component and local index in the component
/// given a continuous index spanning multiple components
/// @param [in] data_glb_idx continuous index covering multiple components
/// @return boost::tuple<data_type::ConstPtr component, Uint idx_in_component>
template <typename DATA>
inline typename CUnifiedData<DATA>::const_data_location_type CUnifiedData<DATA>::location(const Uint data_glb_idx) const
{
  cf_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf_assert(data_vector_idx<m_data_vector.size());
  return boost::make_tuple(m_data_vector[data_vector_idx]->as_const()->as_ptr<DATA>(), data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

/// Get the const component and local index in the component
/// given a continuous index spanning multiple components
/// @param [in] data_glb_idx continuous index covering multiple components
/// @return boost::tuple<Uint component_idx, Uint idx_in_component>
template <typename DATA>
inline boost::tuple<Uint,Uint> CUnifiedData<DATA>::location_idx(const Uint data_glb_idx) const
{
  cf_assert(data_glb_idx<m_size);
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 -  m_data_indices->array().begin();
  cf_assert(data_vector_idx<m_data_vector.size());
  return boost::make_tuple(data_vector_idx, data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

/// Get the total number of data spanning multiple components
/// @return the size
template <typename DATA>
inline Uint CUnifiedData<DATA>::size() const
{
  return m_size;
}

////////////////////////////////////////////////////////////////////////////////

/// non-const access to the unified data components
/// @return vector of data components
template <typename DATA>
inline std::vector<boost::shared_ptr<typename CUnifiedData<DATA>::data_type> >& CUnifiedData<DATA>::data_components()
{
  return m_data_vector;
}

////////////////////////////////////////////////////////////////////////////////

/// const access to the unified data components
/// @return vector of data components
template <typename DATA>
inline const std::vector<boost::shared_ptr<typename CUnifiedData<DATA>::data_type> >& CUnifiedData<DATA>::data_components() const
{
  return m_data_vector;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CUnifiedData_hpp
