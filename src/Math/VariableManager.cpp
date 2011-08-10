// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>

#include "Common/Foreach.hpp"
#include "Common/OptionT.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Math/VariableManager.hpp"
#include "Math/VariablesDescriptor.hpp"

namespace CF {
namespace Math {

using namespace Common;
using namespace Common::XML;

////////////////////////////////////////////////////////////////////////////////

VariableManager::VariableManager(const std::string& name): Component(name)
{
}

VariableManager::~VariableManager()
{
}

VariablesDescriptor& VariableManager::create_descriptor(const std::string& name, const std::string& description)
{
  VariablesDescriptor& result = create_component<VariablesDescriptor>(name);
  result.add_tag(name);

  result.set_variables(description);

  return result;
}

void VariableManager::signal_create_descriptor(SignalArgs& node)
{
  SignalOptions options(node);

  create_descriptor(options.option("name").value_str(), options.option("description").value_str());
}


void VariableManager::signature_create_descriptor(SignalArgs& node)
{
  SignalOptions options(node);

  options.add_option< OptionT<std::string> >("name")
    ->description("Name for the created descriptor, also added to tags")
    ->pretty_name("Name");

  options.add_option< OptionT<std::string> >("description")
    ->description("String to parse into variables")
    ->pretty_name("Description");
}




////////////////////////////////////////////////////////////////////////////////

} // Math
} // CF
