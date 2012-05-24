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
  options().add_option("string", std::string());
  options().add_option("real", 0.);
  options().add_option("uint", 0u);
  options().add_option("int", 0);
  options().add_option("bool", false);
  options().add_option("uri", URI());
  options().add_option("generic_component", Handle<Component>());
  options().add_option("group_component", Handle<Group>());
  options().add_option("string_vector", std::vector<std::string>());
  options().add_option("int_vector", std::vector<int>());
  options().add_option("uint_vector", std::vector<Uint>());
  options().add_option("real_vector", std::vector<Real>());
  options().add_option("bool_vector", std::vector<bool>());
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
