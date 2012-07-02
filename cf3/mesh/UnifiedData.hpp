// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_UnifiedData_hpp
#define cf3_mesh_UnifiedData_hpp

#include <boost/tuple/tuple.hpp>

#include "common/Component.hpp"
#include "common/Group.hpp"
#include "common/Link.hpp"
#include "common/StringConversion.hpp"
#include "common/List.hpp"

#include "mesh/LibMesh.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// This class allows to access data spread over multiple components
/// with a continuous index
/// @pre the data components must be of the same type and must have
///      a member function "Uint size() const" defined.
class Mesh_API UnifiedData : public common::Component
{
public:




  /// Contructor
  /// @param name of the component
  UnifiedData ( const std::string& name );

  /// Virtual destructor
  virtual ~UnifiedData() {}

  /// Get the class name
  static std::string type_name () { return "UnifiedData"; }

  /// @brief add a data component to the unified data table
  /// @param [in] data component
  /// @note The data component must have a "size()" member function defined
  template <typename T>
      void add(T& data);

  /// Get the component and local index in the component
  /// given a continuous index spanning multiple components
  /// @param [in] data_glb_idx continuous index covering multiple components
  /// @return boost::tuple<data_Handle< type > component, Uint idx_in_component>
  boost::tuple<Handle< common::Component >,Uint> location(const Uint data_glb_idx);

  boost::tuple<Handle< common::Component >,Uint> location(const Uint data_glb_idx) const;


  boost::tuple<common::Component&,Uint> location_v2(const Uint data_glb_idx);

  boost::tuple<const common::Component&,Uint> location_v2(const Uint data_glb_idx) const;


  boost::tuple<Uint,Uint> location_idx(const Uint data_glb_idx) const;

  Uint location_comp_idx(const Component& data) const;

  /// Get the total number of data spanning multiple components
  /// @return the size
  Uint size() const;

  /// non-const access to the unified data components
  /// @return vector of data components
  std::vector< Handle<common::Component> >& components();

  /// const access to the unified data components
  /// @return vector of data components
  const std::vector< Handle<common::Component> >& components() const;

  Component& component(const Uint idx) { return *m_data_vector[idx]; }

  const Component& component(const Uint idx) const { return *m_data_vector[idx]; }

  Uint unified_idx(const common::Component& component, const Uint local_idx) const;

  Uint unified_idx(const boost::tuple<Handle< common::Component >,Uint>& loc) const;

  void reset();

  bool contains(const common::Component& data) const;

private: // data

  /// vector of components to view as continuous
  std::vector< Handle<common::Component> > m_data_vector;

  /// start index for each component in the continous view
  Handle<common::List<Uint> > m_data_indices;

  /// group with links to data components (links to elements of m_data_vector)
  Handle<common::Group> m_data_links;

  /// total number of indices spanning all components
  Uint m_size;

  /// map component to start index
  std::map<common::Component const*,Uint> m_start_idx;

}; // UnifiedData

////////////////////////////////////////////////////////////////////////////////

template <typename DATA>
inline void UnifiedData::add(DATA& data)
{
  Handle<DATA> actual_data = Handle<DATA>(data.template handle<Component>());

  // if it is not added yet, add
  if (m_start_idx.find(actual_data.get()) == m_start_idx.end())
  {
    m_start_idx[actual_data.get()] = m_size;

    m_data_links->create_component<common::Link>("data_component_"+common::to_str(m_data_vector.size()))->link_to(*actual_data);

    m_data_vector.push_back(actual_data);
    m_size += actual_data->size();

    common::List<Uint>::Buffer data_start_indices = m_data_indices->create_buffer();
    data_start_indices.add_row(m_size);
    data_start_indices.flush();
  }

  cf3_assert(m_data_vector.size()==m_data_indices->size()-1);
  cf3_assert(m_start_idx.size() == m_data_vector.size());
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_UnifiedData_hpp
