// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/python.hpp>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/Component.hpp"
#include "Common/Option.hpp"
#include "Common/TypeInfo.hpp"

#include "Python/Component.hpp"

namespace CF {
namespace Python {

using namespace boost::python;

// Types that can be held by any
typedef boost::mpl::vector5<std::string, Real, Uint, int, bool> AnyTypes;

struct PythonToAny
{
  PythonToAny(const object& value, boost::any& result, const std::string& target_type, bool& found) :
    m_value(value),
    m_result(result),
    m_target_type(target_type),
    m_found(found)
  {
  }
  
  template<typename T>
  void operator()(T) const
  {
    if(m_found)
      return;
    
    if(Common::class_name_from_typeinfo(typeid(T)) != m_target_type)
      return;
    
    extract<T> extracted_value(m_value);
    if(extracted_value.check())
    {
      m_found = true;
      m_result = extracted_value();
    }
  }
  
  const object& m_value;
  boost::any& m_result;
  const std::string& m_target_type;
  bool& m_found;
};

// Helper functions to convert to any
boost::any python_to_any(const object& val, const std::string& target_type)
{
  boost::any result;
  bool found = false;
  boost::mpl::for_each<AnyTypes>(PythonToAny(val, result, target_type, found));
  
  if(!found)
    throw Common::CastingFailed(FromHere(), "Failed to convert to boost::any");
  
  return result;
}

class ComponentWrapper
{
public:
  ComponentWrapper(Common::Component& component) :
    m_component(component.as_ptr<Common::Component>())
  {
  }
  
  std::string name()
  {
    return component().name();
  }
  
  object create_component(const std::string& name, const std::string& builder_name)
  {
    Common::Component::Ptr built_comp = Common::build_component(builder_name, name);
    component().add_component(built_comp);
    return wrap_component(*built_comp);
  }
  
  void configure_option(const std::string& optname, const object& val)
  {
    Common::Option& option = component().option(optname);
    option.change_value(python_to_any(val, option.data_type()));
  }
  
  std::string option_value_str(const std::string& optname)
  {
    return component().option(optname).value_str();
  }
  
private:
  // checked access
  Common::Component& component()
  {
    if(m_component.expired())
      throw Common::BadPointer(FromHere(), "Wrapped object was deleted");
    
    return *m_component.lock();
  }
  boost::weak_ptr<Common::Component> m_component;
};

object wrap_component(Common::Component& component)
{
  return object(ComponentWrapper(component));
}


void def_component()
{
  class_<ComponentWrapper>("Component", no_init)
    .def("name", &ComponentWrapper::name, "The name of this component")
    .def("create_component", &ComponentWrapper::create_component, "Create a new component, named after the first argument and built using the builder name in the second argument")
    .def("configure_option", &ComponentWrapper::configure_option)
    .def("option_value_str", &ComponentWrapper::option_value_str);
}

} // Python
} // CF
