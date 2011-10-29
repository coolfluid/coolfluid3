// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/Signal.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"

#include "math/VariablesDescriptor.hpp"
#include "math/VariableManager.hpp"

#include "mesh/FieldManager.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/LoadMesh.hpp"
#include "mesh/WriteMesh.hpp"
#include "mesh/Field.hpp"

#include "common/PE/Comm.hpp"

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
    m_component.options().add_option( OptionComponent<VariableManager>::create("variable_manager", &m_variable_manager) )
              ->pretty_name("Variable Manager")
              ->description("Variable manager that is scanned for the tags");
  }

  // Checked access to the variable manager
  VariableManager& variable_manager()
  {
    if(m_variable_manager.expired())
      throw SetupError(FromHere(), "VariableManager not set for FieldManager at " + m_component.uri().string());

    return *m_variable_manager.lock();
  }

  // Signature for the create_fields signal
  void signature_create_fields(SignalArgs& node)
  {
    SignalOptions options(node);

    options.add_option<OptionURI>("field_group")
      ->pretty_name("Field Group")
      ->description("URI for the FieldGroup in which to create fields");

    options.add_option< OptionT<std::string> >("tag")
      ->pretty_name("Tag")
      ->description("Tag for the VariableDescriptors to use and their corresponding fields");
  }

  Component& m_component;
  boost::weak_ptr<VariableManager> m_variable_manager;
};


FieldManager::FieldManager( const std::string& name  ) :
  Component ( name ),
  m_implementation(new Implementation(*this))
{
}

FieldManager::~FieldManager()
{
}


void FieldManager::create_field(const std::string& tag, FieldGroup& field_group)
{
  boost_foreach(VariablesDescriptor& descriptor, find_components_with_tag<VariablesDescriptor>(m_implementation->variable_manager(), tag))
  {
    const Field::ConstPtr existing_field = find_component_ptr_with_tag<Field>(field_group, tag);
    if(is_not_null(existing_field))
    {
      if(descriptor.description() != existing_field->descriptor().description() || descriptor.option(common::Tags::dimension()).value<Uint>() != existing_field->descriptor().option(common::Tags::dimension()).value<Uint>())
      {
        throw SetupError(FromHere(), "Existing field with tag " + tag + " at " + existing_field->uri().string() + " is incompatible with descriptor " + descriptor.uri().string()
              + ": existing " + existing_field->descriptor().description() + " != required " + descriptor.description());
      }

      CFdebug << "Skipping second field creation for tag " << tag << " in fieldgroup " << field_group.uri().string() << CFendl;
      continue;
    }

    field_group.create_field(tag, descriptor).add_tag(tag);
  }
}


void FieldManager::signal_create_field(SignalArgs& node)
{
  SignalOptions options(node);

  const URI field_group_uri = options.option("field_group").value<URI>();

  Component::Ptr field_group_component = access_component_ptr(field_group_uri);
  if(!field_group_component)
    throw ValueNotFound(FromHere(), "No component found at field_group URI: " + field_group_uri.string());

  FieldGroup::Ptr field_group = field_group_component->as_ptr<FieldGroup>();
  if(!field_group)
    throw ValueNotFound(FromHere(), "Wrong component type at field_group URI: " + field_group_uri.string());

  create_field(options.option("tag").value_str(), *field_group);
}

////////////////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
