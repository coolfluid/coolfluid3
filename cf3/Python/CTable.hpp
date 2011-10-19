// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Python_CTable_hpp
#define CF_Python_CTable_hpp

#include <boost/python/object_fwd.hpp>

namespace CF {
  namespace Common { class Component; }
namespace Python {

class ComponentWrapper;

/// Python wrapping for the CTable class
void add_ctable_methods(ComponentWrapper& wrapped, boost::python::api::object& py_obj);

void def_ctable_types();

} // Python
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Python_CTable_hpp
