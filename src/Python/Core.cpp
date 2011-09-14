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
#include "Common/PE/Comm.hpp"

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

  static void initiate(list arglist)
  {
    int argc = len(arglist);
    static char** argv = 0;
    if(!argv)
    {
      argv = new char*[argc];
      for(Uint i = 0; i != argc; ++i)
      {
        std::string arg_i = extract<std::string>(arglist[i]);
        argv[i] = new char[arg_i.size()];
        arg_i.copy(argv[i], arg_i.size());
      }

      Common::Core::instance().initiate(argc, argv);
      Common::PE::Comm::instance().init(argc, argv);
    }
  }
};

void def_core()
{
  class_<Core>("Core", "Core class, the entry point to coolfluid", no_init)
    .def("root", Core::root, "Access to the root of the component tree")
    .staticmethod("root")
    .def("environment", Core::environment, "Access to the environment for setting global options")
    .staticmethod("environment")
    .def("initiate", Core::initiate)
    .staticmethod("initiate");
}

} // Python
} // CF
