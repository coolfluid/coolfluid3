// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_solver_actions_Proto_LSSWrapper_hpp
#define cf3_solver_actions_Proto_LSSWrapper_hpp

#include <boost/proto/core.hpp>

#include "common/List.hpp"
#include "common/Log.hpp"
#include "common/OptionComponent.hpp"

#include "math/LSS/System.hpp"
#include "mesh/Tags.hpp"

/// @file
/// Wraps Component classes for Proto

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Gives access to a component, obtained aither through a linked option or a direct reference in the constructor.
/// Uses a weak pointer internally
/// Implementation class, use the proto-ready terminal type defined below
/// TagT provides an extra tag that can be used by proto to distinguish wrappers of the same type at compile time
template<typename TagT>
class LSSWrapperImpl
{
public:

  typedef TagT tag_type;

  /// Construction using references to the actual component (mainly useful in utests or other non-dynamic code)
  /// Using this constructor does not use dynamic configuration through options
  LSSWrapperImpl(math::LSS::System& component) :
    m_component( new Handle<math::LSS::System>(component.handle<math::LSS::System>()) )
  {
    trigger_component();
  }

  /// Construction using an option that will point to the actual component.
  LSSWrapperImpl(common::Option& component_option) :
    m_component( new Handle<math::LSS::System>() )
  {
    component_option.link_to(m_component.get()).attach_trigger(boost::bind(&LSSWrapperImpl::trigger_component, this));
    trigger_component();
  }

  math::LSS::System& lss()
  {
    cf3_assert(is_not_null(m_component));
    return *m_cached_component;
  }
  
  /// Return the wrappped matrix
  math::LSS::Matrix& matrix()
  {
    cf3_assert(is_not_null(m_component));
    return *m_matrix;
  }
  
  /// Return the wrappped rhs
  math::LSS::Vector& rhs()
  {
    cf3_assert(is_not_null(m_component));
    return *m_rhs;
  }
  
  /// Return the wrappped solution
  math::LSS::Vector& solution()
  {
    cf3_assert(is_not_null(m_component));
    return *m_solution;
  }
  
  /// Convert the given indices, using the given mapping
  template<typename VectorT>
  void convert_to_lss(VectorT& indices)
  {
    if(is_null(m_used_node_map))
      return;
    
    const Uint vec_size = indices.size();
    for(Uint i = 0; i != vec_size; ++i)
      indices[i] = (*m_used_node_map)[indices[i]];
  }
  
  Uint node_to_lss(const Uint node)
  {
    if(is_null(m_used_node_map))
      return node;
    cf3_assert(node < m_used_node_map->size());
    return (*m_used_node_map)[node];
  }

private:
  /// Points to the wrapped component, if any
  /// The shared_ptr wraps the weak_ptr so the link is always OK
  boost::shared_ptr< Handle<math::LSS::System> > m_component;
  math::LSS::System* m_cached_component;
  math::LSS::Matrix* m_matrix;
  math::LSS::Vector* m_rhs;
  math::LSS::Vector* m_solution;
  
  // Used in case there is no 1-to-1 mapping between the mesh nodes and the LSS indices
  common::List<Uint>* m_used_nodes;
  common::List<Uint>* m_used_node_map;
  
  void trigger_component()
  {
    m_cached_component = m_component->get();
    if(is_not_null(m_cached_component))
    {
      m_matrix = m_cached_component->matrix().get();
      m_rhs = m_cached_component->rhs().get();
      m_solution = m_cached_component->solution().get();
      
      m_used_nodes = Handle< common::List<Uint> >(m_cached_component->get_child(mesh::Tags::nodes_used())).get();
      m_used_node_map = Handle< common::List<Uint> >(m_cached_component->get_child("used_node_map")).get();
    }
    else
    {
      m_matrix = nullptr;
      m_rhs = nullptr;
      m_solution = nullptr;
      m_used_node_map = nullptr;
      m_used_nodes = nullptr;
    }
  }
};

/// Proto-ready terminal type for wrapping a component
template<typename TagT>
class LSSWrapper :
  public boost::proto::extends< boost::proto::literal< LSSWrapperImpl<TagT>& >, LSSWrapper<TagT> >
{
public:

  typedef boost::proto::extends< boost::proto::literal< LSSWrapperImpl<TagT>& >, LSSWrapper<TagT> > base_type;

  /// Construction using references to the actual component (mainly useful in utests or other non-dynamic code)
  /// Using this constructor does not use dynamic configuration through options
  LSSWrapper(math::LSS::System& component) :
    m_component_wrapper(component),
    base_type(m_component_wrapper)
  {
  }

  /// Construction using an option that will point to the actual component.
  LSSWrapper(common::Option& component_option) :
    m_component_wrapper(component_option),
    base_type(m_component_wrapper)
  {
  }

private:
  /// Points to the wrapped component, if any
  LSSWrapperImpl<TagT> m_component_wrapper;
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_LSSWrapper_hpp
