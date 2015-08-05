// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "python/BoostPython.hpp"

#include "common/Builder.hpp"
#include "common/ComponentFilter.hpp"
#include "common/Environment.hpp"
#include "common/Libraries.hpp"
#include "common/Group.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "python/ComponentFilterPython.hpp"
#include "python/ComponentWrapper.hpp"
#include "python/LibPython.hpp"
#include "python/Utility.hpp"

namespace cf3 {
namespace python {

class ComponentFilterPython : public common::ComponentFilter
{
public:
  /// Contructor
  /// @param name of the component
  ComponentFilterPython ( const std::string& name ) : ComponentFilter(name)
  {
  }

  /// Get the class name
  static std::string type_name () { return "ComponentFilterPython"; }

  /// Implement this to return true or false depending on the criteria of the filter
  virtual bool operator()(const Component& component)
  {
    if(m_callable.is_none())
    {
      PyErr_SetString(PyExc_RuntimeError, "Python filter object is not set");
      boost::python::throw_error_already_set();
    }

    return m_callable(wrap_component(const_cast<common::Component*>(&component)->handle()));
  }

  void set_callable(boost::python::object callable)
  {
    if(PyCallable_Check(callable.ptr()) != 1)
    {
      PyErr_SetString(PyExc_TypeError, "Python filter object is not callable");
      boost::python::throw_error_already_set();
    }

    m_callable = callable;

  }

private:
  boost::python::object m_callable;
};

common::ComponentBuilder < ComponentFilterPython, common::ComponentFilter, LibPython > ComponentFilterPython_Builder;

class ComponentFilterPythonWrapper : public ComponentWrapper
{
public:
  ComponentFilterPythonWrapper(const Handle<ComponentFilterPython>& component) : ComponentWrapper(component)
  {
  }
};

class ComponentFilterPythonFactory : public ComponentWrapperFactory
{
  virtual boost::python::object wrap_component(const cf3::Handle<common::Component>& component) const
  {
    Handle<ComponentFilterPython> filter(component);
    if(is_null(filter))
      return boost::python::object();

    return boost::python::object(ComponentFilterPythonWrapper(filter));
  }
};

bool component_filter_python_call(ComponentFilterPythonWrapper& self, ComponentWrapper& component)
{
  return self.component<ComponentFilterPython>()(component.component());
}

void component_filter_python_set_callable(ComponentFilterPythonWrapper& self, boost::python::object callable)
{
  self.component<ComponentFilterPython>().set_callable(callable);
}

void def_component_filter_methods()
{
  boost::python::class_<ComponentFilterPythonWrapper, boost::python::bases<ComponentWrapper> >("Component", boost::python::no_init)
    .def("__call__", component_filter_python_call, "Apply the filter")
    .def("set_callable", component_filter_python_set_callable, "Set the Python callable to use when applying the filter");

  ComponentWrapperRegistry::instance().register_factory<ComponentFilterPythonFactory>();
}

} // python
} // cf3
