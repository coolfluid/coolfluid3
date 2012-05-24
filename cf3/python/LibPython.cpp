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

LibPython::~LibPython()
{
  if(m_is_initiated)
    terminate_impl();
}


////////////////////////////////////////////////////////////////////////////////

void LibPython::initiate()
{
  if(m_is_initiated)
    return;

  initiate_impl();
  m_is_initiated = true;
}


void LibPython::terminate()
{
  if(!m_is_initiated)
    return;

  terminate_impl();
  m_is_initiated = false;
}


void LibPython::initiate_impl()
{
//   Handle<common::Component> group = common::Core::instance().tools().create_component("Python", "cf3.common.Group");
//   group->create_component("ScriptEngine", "cf3.python.ScriptEngine");
}

void LibPython::terminate_impl()
{
//   common::Core::instance().tools().remove_component("Python");
}

////////////////////////////////////////////////////////////////////////////////

} // python
} // cf3
