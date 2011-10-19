// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CGroup.hpp"
#include "Common/RegistLibrary.hpp"

#include "Python/LibPython.hpp"

namespace cf3 {
namespace Python {

cf3::common::RegistLibrary<LibPython> libPython;

////////////////////////////////////////////////////////////////////////////////

void LibPython::initiate_impl()
{
  common::Component& group = common::Core::instance().tools().create_component("Python", "CF.Common.CGroup");
  group.create_component("ScriptEngine", "CF.Python.ScriptEngine");
}

void LibPython::terminate_impl()
{
  common::Component& group = common::Core::instance().tools().get_child("Python");
  group.remove_component("ScriptEngine");
  common::Core::instance().tools().remove_component("Python");
}

////////////////////////////////////////////////////////////////////////////////

} // Python
} // cf3
