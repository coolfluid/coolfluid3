// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Group.hpp"
#include "common/RegistLibrary.hpp"

#include "python/LibPython.hpp"

namespace cf3 {
namespace python {

cf3::common::RegistLibrary<LibPython> libPython;

////////////////////////////////////////////////////////////////////////////////

void LibPython::initiate_impl()
{
  common::Component& group = common::Core::instance().tools().create_component("Python", "cf3.common.Group");
  group.create_component("ScriptEngine", "cf3.python.ScriptEngine");
}

void LibPython::terminate_impl()
{
  common::Component& group = common::Core::instance().tools().get_child("Python");
  group.remove_component("ScriptEngine");
  common::Core::instance().tools().remove_component("Python");
}

////////////////////////////////////////////////////////////////////////////////

} // python
} // cf3
