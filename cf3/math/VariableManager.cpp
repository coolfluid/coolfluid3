// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>

#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/OptionT.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

namespace cf3 {
namespace math {

using namespace common;
using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < VariableManager, Component, LibMath > VariableManager_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

VariableManager::VariableManager(const std::string& name): Component(name)
{
}

VariableManager::~VariableManager()
{
}

VariablesDescriptor& VariableManager::create_descriptor(const std::string& name, const std::string& description)
{
  Handle<VariablesDescriptor> result = create_component<VariablesDescriptor>(name);
  result->add_tag(name);

  result->set_variables(description);

  return *result;
}

void VariableManager::signal_create_descriptor(SignalArgs& node)
{
  SignalOptions options(node);

  create_descriptor(options.option("name").value_str(), options.option("description").value_str());
}


void VariableManager::signature_create_descriptor(SignalArgs& node)
{
  SignalOptions options(node);

  options.add<std::string>("name")
    .description("Name for the created descriptor, also added to tags")
    .pretty_name("Name");

  options.add<std::string>("description")
    .description("String to parse into variables")
    .pretty_name("Description");
}




////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3
