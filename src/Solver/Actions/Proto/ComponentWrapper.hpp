// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Solver_Actions_Proto_ComponentWrapper_hpp
#define CF_Solver_Actions_Proto_ComponentWrapper_hpp

#include <boost/proto/core.hpp>

#include "Common/Log.hpp"
#include "Common/OptionComponent.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Physics/PhysModel.hpp"

/// @file
/// Wraps Component classes for Proto

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Gives access to a component, obtained aither through a linked option or a direct reference in the constructor.
/// Uses a weak pointer internally
/// Non-copyable because the link at construction-time must be preserved.
/// Implementation class, use the proto-ready terminal type defined below
template<typename ComponentT>
class ComponentWrapperImpl :
  boost::noncopyable
{
public:

  /// Construction using references to the actual component (mainly useful in utests or other non-dynamic code)
  /// Using this constructor does not use dynamic configuration through options
  ComponentWrapperImpl(ComponentT& component) :
    m_component(component.template as_ptr<ComponentT>())
  {
  }

  /// Construction using an option that will point to the actual component.
  ComponentWrapperImpl(Common::Option& component_option)
  {
    component_option.link_to(&m_component);
  }

  ~ComponentWrapperImpl()
  {
    // TODO: Remove link created at construction time here
    CFdebug << "Destroying component wrapper, possible dangling link" << CFendl;
  }

  /// Return the wrapped component
  ComponentT& component()
  {
    if(m_component.expired())
      throw Common::SetupError(FromHere(), "ComponentWrapperImpl points to a null component");
    
    return *m_component.lock();
  }

private:
  /// Points to the wrapped component, if any
  boost::weak_ptr<ComponentT> m_component;
};

/// Proto-ready terminal type for wrapping a component
/// TagT provides an extra tag that can be used by proto to distinguish wrappers of the same type at compile time
template<typename ComponentT, typename TagT>
class ComponentWrapper :
  public boost::proto::extends< boost::proto::literal< ComponentWrapperImpl<ComponentT>& >, ComponentWrapper<ComponentT, TagT> >
{
public:

  typedef boost::proto::extends< boost::proto::literal< ComponentWrapperImpl<ComponentT>& >, ComponentWrapper<ComponentT, TagT> > base_type;

  /// Construction using references to the actual component (mainly useful in utests or other non-dynamic code)
  /// Using this constructor does not use dynamic configuration through options
  ComponentWrapper(ComponentT& component) :
    m_component_wrapper(component),
    base_type(m_component_wrapper)
  {
  }

  /// Construction using an option that will point to the actual component.
  ComponentWrapper(Common::Option& component_option) :
    m_component_wrapper(component_option),
    base_type(m_component_wrapper)
  {
  }

private:
  /// Points to the wrapped component, if any
  ComponentWrapperImpl<ComponentT> m_component_wrapper;
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ComponentWrapper_hpp
