// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF3_Python_Table_hpp
#define CF3_Python_Table_hpp

#include <boost/python/object_fwd.hpp>

namespace cf3 {
namespace common { class Component; }
namespace python {

class ComponentWrapper;

/// Python wrapping for the Table class
void add_ctable_methods(ComponentWrapper& wrapped, boost::python::api::object& py_obj);

void def_ctable_types();

} // python
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Python_Table_hpp
