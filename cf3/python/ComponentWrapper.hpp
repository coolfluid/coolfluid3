// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Python_Component_hpp
#define cf3_Python_Component_hpp

#include <boost/python/object.hpp>

#include "common/BasicExceptions.hpp"
#include "common/SignalHandler.hpp"

namespace cf3 {
  namespace common { class Component; }
  template<typename T> class Handle;
namespace python {

/// Encapsulates the behavior required for Python lists. When wrapping a component for Python,
/// these functions will always be defined, but they throw exceptions when list functions are called on components
/// that can't behave like a list. This is necessary because adding methods like __len__ dynamically
/// can't work, because Python expects these to be defined at the level of the class, not just at the level
/// of the object
struct PythonListInterface
{
  virtual ~PythonListInterface() {}

  /// The number of items in the list
  virtual Uint len() const = 0;

  /// Get item i from the list
  virtual boost::python::object get_item(const Uint i) const = 0;

  /// Set item i in the list
  virtual void set_item(const Uint i, boost::python::object& value) = 0;

  /// Get the whole list as a string
  virtual std::string to_str() const = 0;
};
  
/// Wrapper class for components
class ComponentWrapper
{
public:
  ComponentWrapper(const Handle<common::Component>& component);
  ~ComponentWrapper();

  /// Access to the wrapped component
  common::Component& component();

  /// Properly typed access to the wrapped component
  template<typename ComponentT>
  ComponentT& component()
  {
    ComponentT* comp = dynamic_cast<ComponentT*>(&component());
    if(!comp)
      throw common::CastingFailed(FromHere(), "Could not cast python wrapped object to type " + ComponentT::type_name());

    return *comp;
  }

  /// Set the list interface to use. Takes ownership of the passed pointer.
  /// List interfaces are used to override the python [] operator for certain components,
  /// such as tables and lists
  void set_list_interface(PythonListInterface* interface);

  /// Get the curent list interface, or null if there is none
  PythonListInterface* get_list_interface();

private:
  friend boost::python::object wrap_component(const cf3::Handle<common::Component>&);
  class Implementation;
  boost::shared_ptr<Implementation> m_implementation; // Shared pointer so we can easily make shallow copies
};

/// Python wrapping for the Component class
void def_component();

/// Wrap the passed component in a python object
boost::python::object wrap_component(const cf3::Handle<common::Component>& component);

} // python
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Python_Component_hpp
