// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Signal.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"

#include "Math/VariablesDescriptor.hpp"
#include "Math/VariableManager.hpp"

#include "Mesh/FieldManager.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/LoadMesh.hpp"
#include "Mesh/WriteMesh.hpp"

#include "Common/MPI/PE.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;
using namespace Common::mpi;
using namespace Math;



Common::ComponentBuilder < FieldManager, Component, LibMesh > FieldManager_Builder;

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

////////////////////////////////////////////////////////////////////////////////////////////

FieldManager::FieldManager( const std::string& name  ) :
  Component ( name ),
  m_implementation(new Implementation(*this))
{
}

FieldManager::~FieldManager()
{
}

////////////////////////////////////////////////////////////////////////////////

void FieldManager::create_fields(const std::string& tag, FieldGroup& field_group)
{
  boost_foreach(const VariablesDescriptor& descriptor, find_components_with_tag<VariablesDescriptor>(m_implementation->variable_manager(), tag))
  {
    if(find_component_ptr_with_tag(field_group, tag)) // TODO: Check if the tagged fields that exist have the same variable descriptor
    {
      CFdebug << "Skipping second field creation for tag " << tag << " in fieldgroup " << field_group.uri().string() << CFendl;
      continue;
    }

    field_group.create_field(tag, descriptor.description()).add_tag(tag);
  }
}

void FieldManager::create_fields(const std::string& tag, CMesh& mesh, const CField::Basis::Type base, const std::string& space)
{
  boost_foreach(const VariablesDescriptor& descriptor, find_components_with_tag<VariablesDescriptor>(m_implementation->variable_manager(), tag))
  {
    if(find_component_ptr_with_tag(mesh, tag))
    {
      CFdebug << "Skipping second field creation for tag " << tag << " in mesh " << mesh.uri().string() << CFendl;
      continue;
    }

    mesh.create_field(tag, base, space, descriptor.description()).add_tag(tag);
  }
}


////////////////////////////////////////////////////////////////////////////////

void FieldManager::signal_create_fields(SignalArgs& node)
{
  SignalOptions options(node);

  const URI field_group_uri = options.option("field_group").value<URI>();

  Component::Ptr field_group_component = access_component_ptr(field_group_uri);
  if(!field_group_component)
    throw ValueNotFound(FromHere(), "No component found at field_group URI: " + field_group_uri.string());

  FieldGroup::Ptr field_group = field_group_component->as_ptr<FieldGroup>();
  if(!field_group)
    throw ValueNotFound(FromHere(), "Wrong component type at field_group URI: " + field_group_uri.string());

  create_fields(options.option("tag").value_str(), *field_group);
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
