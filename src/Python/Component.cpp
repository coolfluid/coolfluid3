// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/python.hpp>

#include "Common/Component.hpp"

#include "Python/Component.hpp"

namespace CF {
namespace Python {

using namespace boost::python;

class ComponentWrapper
{
public:
  ComponentWrapper(Common::Component& component) :
    m_component(component.as_ptr<Common::Component>())
  {
  }
  
  std::string name()
  {
    return m_component.lock()->name();
  }
  
private:
  boost::weak_ptr<Common::Component> m_component;
};

object wrap_component(Common::Component& component)
{
  return object(ComponentWrapper(component));
}


void def_component()
{
  class_<ComponentWrapper>("Component", no_init)
    .def("name", &ComponentWrapper::name);
}

} // Python
} // CF
