// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF3_Python_Utility_hpp
#define CF3_Python_Utility_hpp

#include "python/BoostPython.hpp"

#include <string>

namespace cf3 {
namespace python {

/// Helper function to set attributes, since we override the __setattr__ function. This behaves exactly like boost::python::setattr,
/// except that it calls the generic python setattr and thus does not recurse on components
inline void generic_setattr(boost::python::object const& target, char const* key, boost::python::object const& value)
{
  boost::python::str key_str(key);
  if(PyObject_GenericSetAttr(target.ptr(), key_str.ptr(), value.ptr()) == -1)
    boost::python::throw_error_already_set();
}
  
/// Helper function to get a weak reference to the given object
inline boost::python::object weak_ref(const boost::python::object& source)
{
  return boost::python::object(boost::python::handle<>(PyWeakref_NewRef(source.ptr(), NULL)));
}
  
/// Add a function dynamically
/// @param object Object to add a function to
/// @param function Function to add (function pointer, ...)
/// @param name Name of the function in python
/// @param docstring Python documentation for the function
template<typename FunctionT>
void add_function(boost::python::object& object, FunctionT function, const std::string& name, const std::string& docstring = "")
{
  boost::python::object func_obj = boost::python::make_function(function);
  boost::python::setattr(func_obj, "__doc__", boost::python::str(docstring.c_str()));
  // TODO: weak ref here doesn't work for some reasons
  generic_setattr(object, name.c_str(), boost::python::import("types").attr("MethodType")(func_obj, object));
}

} // python
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Python_Utility_hpp
