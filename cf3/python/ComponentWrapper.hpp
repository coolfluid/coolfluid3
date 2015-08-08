// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Python_Component_hpp
#define cf3_Python_Component_hpp

#include <boost/python/object.hpp>
#include <boost/type_traits/is_const.hpp>

#include "common/BasicExceptions.hpp"
#include "common/Component.hpp"
#include "common/Foreach.hpp"
#include "common/SignalHandler.hpp"

namespace cf3 {
  namespace common { class Component; }
  template<typename T> class Handle;
namespace python {

/// Base class for ComponentWrapper
class ComponentWrapperBase
{
public:
  ComponentWrapperBase(const Handle<common::Component const>& component);
  virtual ~ComponentWrapperBase();

  virtual const common::Component& component() const = 0;

  /// Properly typed access to the wrapped component
  template<typename CastComponentT>
  const CastComponentT& component() const
  {
    const CastComponentT* comp = dynamic_cast<const CastComponentT*>(&component());
    if(!comp)
      throw common::CastingFailed(FromHere(), "Could not cast python wrapped object to type " + CastComponentT::type_name());

    return *comp;
  }

  /// Bind the signals of the component to the given Python object
  void bind_signals(boost::python::object& python_object);
private:
  class Implementation;
  boost::shared_ptr<Implementation> m_implementation; // Shared pointer so we can easily make shallow copies
};

/// Wrapper class for components
template<typename ComponentT>
class ComponentWrapperT : public ComponentWrapperBase
{
public:
  explicit ComponentWrapperT(const Handle<ComponentT>& component) : ComponentWrapperBase(component), m_component(component)
  {
  }

  template<typename OtherComponentT>
  ComponentWrapperT(const ComponentWrapperT<OtherComponentT>& component_wrapper) : ComponentWrapperBase(component_wrapper.m_component), m_component(component_wrapper.m_component)
  {
  }

  /// Access to the wrapped component
  ComponentT& component()
  {
    if(is_null(m_component))
      throw common::BadPointer(FromHere(), "Wrapped object was deleted");

    return *m_component;
  }

  virtual const common::Component& component() const
  {
    if(is_null(m_component))
      throw common::BadPointer(FromHere(), "Wrapped object was deleted");

    return *m_component;
  }

  /// Properly typed access to the wrapped component
  template<typename CastComponentT>
  CastComponentT& component()
  {
    CastComponentT* comp = dynamic_cast<CastComponentT*>(&component());
    if(!comp)
      throw common::CastingFailed(FromHere(), "Could not cast python wrapped object to type " + CastComponentT::type_name());

    return *comp;
  }

private:
  Handle<ComponentT> m_component;
  template <class> friend class ComponentWrapperT;
};

typedef ComponentWrapperT<common::Component> ComponentWrapper;
typedef ComponentWrapperT<common::Component const> ComponentWrapperConst;

/// Boiler-plate implementation of a derived component wrapper, non-const version
template<typename ComponentT>
class DerivedComponentWrapper : public ComponentWrapper
{
public:
  DerivedComponentWrapper(const Handle<ComponentT>& component) : ComponentWrapper(component)
  {
  }
};

/// Boiler-plate implementation of a derived coponent wrapper, const version
template<typename ComponentT>
class DerivedComponentWrapper<const ComponentT> : public ComponentWrapperConst
{
public:
  DerivedComponentWrapper(const Handle<ComponentT const>& component) : ComponentWrapperConst(component)
  {
  }

  /// Conversion from the non-const version
  DerivedComponentWrapper(const DerivedComponentWrapper<ComponentT>& other_wrapper) : ComponentWrapperConst(other_wrapper.component().handle())
  {
  }
};

/// Factory for component wrappers that expand functionality
class ComponentWrapperFactory
{
public:
  virtual boost::python::object wrap_component(common::Component& component) const = 0;
  virtual boost::python::object wrap_component(const common::Component& component) const = 0;
};

/// Default implementation of a factory that simply checks the type of the component and uses the standard DerivedComponentWrapper
template<typename ComponentT>
class DefaultComponentWrapperFactory : public ComponentWrapperFactory
{
public:
  template<typename TargetComponentT, typename SourceComponentT>
  boost::python::object wrap_component_template(SourceComponentT& component) const
  {
    Handle<TargetComponentT> result(component.handle());
    if(is_null(result))
      return boost::python::object();

    return boost::python::object(DerivedComponentWrapper<TargetComponentT>(result));
  }

  virtual boost::python::object wrap_component(common::Component& component) const
  {
    return wrap_component_template<ComponentT>(component);
  }

  virtual boost::python::object wrap_component(const common::Component& component) const
  {
    return wrap_component_template<ComponentT const>(component);
  }
};

/// Keeps track of specialized factories for ComponentWrappers that add functionality
class ComponentWrapperRegistry
{
public:
  static ComponentWrapperRegistry& instance();

  template<typename FactoryT>
  void register_factory()
  {
    m_factories.push_back(boost::shared_ptr<ComponentWrapperFactory>(new FactoryT()));
  }

  template<typename ComponentT>
  boost::python::object wrap_component(ComponentT& component) const
  {
    boost::python::object result;

    BOOST_FOREACH(const boost::shared_ptr<ComponentWrapperFactory>& factory, m_factories)
    {
      result = factory->wrap_component(component);
      if(!result.is_none())
        return result;
    }

    return result;
  }

private:
  ComponentWrapperRegistry()
  {
  }

  std::vector< boost::shared_ptr<ComponentWrapperFactory> > m_factories;
};

/// Wrap the passed component in a python object
template<typename ComponentT>
boost::python::object wrap_component(const cf3::Handle<ComponentT>& component)
{
  if(is_null(component))
    return boost::python::object();

  boost::python::object result = ComponentWrapperRegistry::instance().wrap_component(*component);
  if(result.is_none())
  {
    result = boost::python::object(ComponentWrapperT<ComponentT>(component));
  }
  ComponentWrapperT<ComponentT>& wrapped = boost::python::extract<ComponentWrapperT<ComponentT>&>(result);
  wrapped.bind_signals(result);

  cf3_assert(result.ptr()->ob_refcnt == 1); // Make sure there are no circular references

  return result;
}

/// Python wrapping for the Component class
void def_component();

} // python
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Python_Component_hpp
