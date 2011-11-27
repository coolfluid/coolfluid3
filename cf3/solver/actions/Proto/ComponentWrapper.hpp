// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_solver_actions_Proto_ComponentWrapper_hpp
#define cf3_solver_actions_Proto_ComponentWrapper_hpp

#include <boost/proto/core.hpp>

#include "common/Log.hpp"
#include "common/OptionComponent.hpp"

#include "physics/PhysModel.hpp"

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
template<typename ComponentT, typename TagT>
class ComponentWrapperImpl
{
public:

  typedef TagT tag_type;

  /// Construction using references to the actual component (mainly useful in utests or other non-dynamic code)
  /// Using this constructor does not use dynamic configuration through options
  ComponentWrapperImpl(ComponentT& component) :
    m_component( new Handle<ComponentT>(component.template handle<ComponentT>()) )
  {
  }

  /// Construction using an option that will point to the actual component.
  ComponentWrapperImpl(common::Option& component_option) :
    m_component( new Handle<ComponentT>() )
  {
    component_option.link_to(m_component.get());
  }

  /// Return the wrapped component
  ComponentT& component()
  {
    if(m_component->expired())
      throw common::SetupError(FromHere(), "ComponentWrapperImpl points to a null component");

    return *(m_component->lock());
  }

private:
  /// Points to the wrapped component, if any
  /// The shared_ptr wraps the weak_ptr so the link is always OK
  boost::shared_ptr< Handle<ComponentT> > m_component;
};

/// Proto-ready terminal type for wrapping a component
template<typename ComponentT, typename TagT>
class ComponentWrapper :
  public boost::proto::extends< boost::proto::literal< ComponentWrapperImpl<ComponentT, TagT>& >, ComponentWrapper<ComponentT, TagT> >
{
public:

  typedef boost::proto::extends< boost::proto::literal< ComponentWrapperImpl<ComponentT, TagT>& >, ComponentWrapper<ComponentT, TagT> > base_type;

  /// Construction using references to the actual component (mainly useful in utests or other non-dynamic code)
  /// Using this constructor does not use dynamic configuration through options
  ComponentWrapper(ComponentT& component) :
    m_component_wrapper(component),
    base_type(m_component_wrapper)
  {
  }

  /// Construction using an option that will point to the actual component.
  ComponentWrapper(common::Option& component_option) :
    m_component_wrapper(component_option),
    base_type(m_component_wrapper)
  {
  }

private:
  /// Points to the wrapped component, if any
  ComponentWrapperImpl<ComponentT, TagT> m_component_wrapper;
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_ComponentWrapper_hpp
