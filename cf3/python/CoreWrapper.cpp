// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "python/BoostPython.hpp"

#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/Libraries.hpp"
#include "common/Group.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "python/CoreWrapper.hpp"
#include "python/ComponentWrapper.hpp"

namespace cf3 {
namespace python {

struct CoreWrapper
{
  static boost::python::object root()
  {
    return wrap_component(common::Core::instance().root().handle<common::Component>());
  }

  static boost::python::object environment()
  {
    return wrap_component(common::Core::instance().environment().handle<common::Component>());
  }

  static boost::python::object libraries()
  {
    return wrap_component(common::Core::instance().libraries().handle<common::Component>());
  }

  static boost::python::object tools()
  {
    return wrap_component(common::Core::instance().tools().handle<common::Component>());
  }

  static void initiate(boost::python::list arglist)
  {
    int argc = len(arglist);
    static char** argv = 0;
    if(!argv)
    {
      argv = new char*[argc];
      for(Uint i = 0; i != argc; ++i)
      {
        std::string arg_i = boost::python::extract<std::string>(arglist[i]);
        argv[i] = new char[arg_i.size()];
        arg_i.copy(argv[i], arg_i.size());
      }

      common::Core::instance().initiate(argc, argv);
      common::PE::Comm::instance().init(argc, argv);
    }
  }

  static Uint rank()
  {
    return common::PE::Comm::instance().rank();
  }

  static Uint nb_procs()
  {
    return common::PE::Comm::instance().size();
  }

  static void terminate()
  {
    common::Core::instance().terminate();
  }
  
  static void wait_for_debugger(const int rank)
  {
    common::PE::wait_for_debugger(rank);
  }
};

void def_core()
{
  boost::python::class_<CoreWrapper>("Core", "Core class, the entry point to coolfluid", boost::python::no_init)
    .def("root", CoreWrapper::root, "Access to the root of the component tree")
    .staticmethod("root")
    .def("environment", CoreWrapper::environment, "Access to the environment for setting global options")
    .staticmethod("environment")
    .def("libraries", CoreWrapper::libraries, "Access to the environment for setting global options")
    .staticmethod("libraries")
    .def("tools", CoreWrapper::tools, "Access to the tools")
    .staticmethod("tools")
    .def("initiate", CoreWrapper::initiate)
    .staticmethod("initiate")
    .def("terminate", CoreWrapper::terminate)
    .staticmethod("terminate")
    .def("rank", CoreWrapper::rank)
    .staticmethod("rank")
    .def("nb_procs", CoreWrapper::nb_procs)
    .staticmethod("nb_procs")
    .def("wait_for_debugger", CoreWrapper::wait_for_debugger)
    .staticmethod("wait_for_debugger");
}


} // python
} // cf3
