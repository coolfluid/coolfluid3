// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Group.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/URI.hpp"

#include "python/LibPython.hpp"
#include "python/TestAllOptions.hpp"

namespace cf3 {
namespace python {

using namespace common;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < TestAllOptions, Component, LibPython > TestAllOptions_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

TestAllOptions::TestAllOptions ( const std::string& name ) : Component ( name )
{
  options().add("string", std::string()).description("Test string option").mark_basic();
  options().add("real", 0.);
  options().add("uint", 0u);
  options().add("int", 0);
  options().add("bool", false);
  options().add("uri", URI());
  options().add("generic_component", Handle<Component>());
  options().add("const_component", Handle<Component const>()).mark_basic();
  options().add("group_component", Handle<Group>());
  options().add("string_vector", std::vector<std::string>());
  options().add("int_vector", std::vector<int>());
  options().add("uint_vector", std::vector<Uint>());
  options().add("real_vector", std::vector<Real>());
  options().add("bool_vector", std::vector<bool>());
  options().add("generic_component_vector", std::vector< Handle<Component> >());
  options().add("group_component_vector", std::vector< Handle<Group> >());
}


TestAllOptions::~TestAllOptions()
{
}

void TestAllOptions::trigger_debug()
{
  CFdebug << "new uri set: " << options()["uri"].value<URI>().path() << CFendl;
}


////////////////////////////////////////////////////////////////////////////////////////////

} // python
} // cf3
