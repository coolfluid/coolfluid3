// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Python_PythonAny_hpp
#define cf3_Python_PythonAny_hpp

#include <boost/python/object.hpp>
#include <boost/any.hpp>

namespace cf3 {
namespace python {

/// Convert the given any to a python object
boost::python::object any_to_python(const boost::any& value);

/// Convert the given python object to any
boost::any python_to_any(const boost::python::object& value);

/// Return the coolfluid type string for the given python type
std::string type_name(const boost::python::object& python_object);

} // python
} // cf3

  ////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Python_PythonAny_hpp
