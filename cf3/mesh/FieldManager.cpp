// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/Signal.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"

#include "math/VariablesDescriptor.hpp"
#include "math/VariableManager.hpp"

#include "mesh/FieldManager.hpp"
#include "mesh/Field.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;
using namespace math;


////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < FieldManager, Component, LibMesh > FieldManager_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

struct FieldManager::Implementation
{
  Implementation(Component& component) :
    m_component(component)
  {
    m_component.options().add("variable_manager", m_variable_manager)
              .pretty_name("Variable Manager")
              .description("Variable manager that is scanned for the tags")
              .link_to(&m_variable_manager);
  }

  // Checked access to the variable manager
  VariableManager& variable_manager()
  {
    if(is_null(m_variable_manager))
      throw SetupError(FromHere(), "VariableManager not set for FieldManager at " + m_component.uri().string());

    return *m_variable_manager;
  }

  // Signature for the create_fields signal
  void signature_create_fields(SignalArgs& node)
  {
    SignalOptions options(node);

    options.add<URI>("dict")
      .pretty_name("Field Group")
      .description("URI for the Dictionary in which to create fields");

    options.add<std::string>("tag")
      .pretty_name("Tag")
      .description("Tag for the VariableDescriptors to use and their corresponding fields");
  }

  Component& m_component;
  Handle<VariableManager> m_variable_manager;
};


FieldManager::FieldManager( const std::string& name  ) :
  Component ( name ),
  m_implementation(new Implementation(*this))
{
}

FieldManager::~FieldManager()
{
}


void FieldManager::create_field(const std::string& tag, Dictionary& dict)
{
  boost_foreach(VariablesDescriptor& descriptor, find_components_with_tag<VariablesDescriptor>(m_implementation->variable_manager(), tag))
  {
    const Handle< Field > existing_field = find_component_ptr_with_tag<Field>(dict, tag);
    if(is_not_null(existing_field))
    {
      if(descriptor.description() != existing_field->descriptor().description() || descriptor.options().option(common::Tags::dimension()).value<Uint>() != existing_field->descriptor().options().option(common::Tags::dimension()).value<Uint>())
      {
        throw SetupError(FromHere(), "Existing field with tag " + tag + " at " + existing_field->uri().string() + " is incompatible with descriptor " + descriptor.uri().string()
              + ": existing " + existing_field->descriptor().description() + " != required " + descriptor.description());
      }

      CFdebug << "Skipping second field creation for tag " << tag << " in fieldgroup " << dict.uri().string() << CFendl;
      continue;
    }

    dict.create_field(tag, descriptor).add_tag(tag);
    
    CFdebug << "Creating field with tag " << tag << ": " << descriptor.description() << CFendl;
  }
}


void FieldManager::signal_create_field(SignalArgs& node)
{
  SignalOptions options(node);

  const URI dict_uri = options.option("dict").value<URI>();

  Handle< Component > dict_component = access_component(dict_uri);
  if(!dict_component)
    throw ValueNotFound(FromHere(), "No component found at dict URI: " + dict_uri.string());

  Handle< Dictionary > dict = Handle<Dictionary>(dict_component);
  if(!dict)
    throw ValueNotFound(FromHere(), "Wrong component type at dict URI: " + dict_uri.string());

  create_field(options.option("tag").value_str(), *dict);
}

////////////////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
