// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Python_Utility_hpp
#define CF_Python_Utility_hpp

#include <boost/python/import.hpp>
#include <boost/python/make_function.hpp>
#include <boost/python/object.hpp>
#include <boost/python/str.hpp>

#include <string>

namespace CF {
namespace Python {

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
  boost::python::setattr(object, name.c_str(), boost::python::import("types").attr("MethodType")(func_obj, object));
}

} // Python
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Python_Utility_hpp
