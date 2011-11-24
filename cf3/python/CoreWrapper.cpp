// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/weak_ptr.hpp>

#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/PE/Comm.hpp"

#include "python/BoostPython.hpp"
#include "python/CoreWrapper.hpp"
#include "python/ComponentWrapper.hpp"

namespace cf3 {
namespace python {

using namespace boost::python;

struct CoreWrapper
{
  static object root()
  {
    return wrap_component(common::Core::instance().root());
  }

  static object environment()
  {
    return wrap_component(common::Core::instance().environment());
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

      common::Core::instance().initiate(argc, argv);
      common::PE::Comm::instance().init(argc, argv);
    }
  }
  
  static void terminate()
  {
    common::Core::instance().terminate();
  }
};

void def_core()
{
  class_<CoreWrapper>("Core", "Core class, the entry point to coolfluid", no_init)
    .def("root", CoreWrapper::root, "Access to the root of the component tree")
    .staticmethod("root")
    .def("environment", CoreWrapper::environment, "Access to the environment for setting global options")
    .staticmethod("environment")
    .def("initiate", CoreWrapper::initiate)
    .staticmethod("initiate")
    .def("terminate", CoreWrapper::terminate)
    .staticmethod("terminate");
}

} // python
} // cf3
