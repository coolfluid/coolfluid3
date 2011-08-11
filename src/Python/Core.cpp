// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/python.hpp>
#include <boost/weak_ptr.hpp>

#include "Common/Core.hpp"
#include "Common/CEnv.hpp"
#include "Common/CRoot.hpp"

#include "Python/Core.hpp"
#include "Python/Component.hpp"

namespace CF {
namespace Python {

using namespace boost::python;

struct Core
{
  static object root()
  {
    return wrap_component(Common::Core::instance().root());
  }

  static object environment()
  {
    return wrap_component(Common::Core::instance().environment());
  }
};

void def_core()
{
  class_<Core>("Core", "Core class, the entry point to coolfluid", no_init)
    .def("root", Core::root, "Access to the root of the component tree")
    .staticmethod("root")
    .def("environment", Core::environment, "Access to the environment for setting global options")
    .staticmethod("environment");
}

} // Python
} // CF
