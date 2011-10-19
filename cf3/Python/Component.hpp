// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Python_Component_hpp
#define cf3_Python_Component_hpp

#include <boost/python/object.hpp>

#include "Python/LibPython.hpp"

namespace cf3 {
  namespace common { class Component; }
namespace Python {

/// Python wrapping for the Component class
void Python_API def_component();

boost::python::object wrap_component(common::Component& component);

} // Python
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Python_Component_hpp
