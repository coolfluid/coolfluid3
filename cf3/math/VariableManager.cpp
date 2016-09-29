// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
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
#include "common/Signal.hpp"
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
  regist_signal( "create_descriptor" )
    .connect( boost::bind( &VariableManager::signal_create_descriptor, this, _1 ) )
    .description("Create a new descriptor")
    .pretty_name("Create Descriptor")
    .signature( boost::bind ( &VariableManager::signature_create_descriptor, this, _1) );
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

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", create_descriptor(options.option("name").value_str(), options.option("description").value_str()).uri());
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
