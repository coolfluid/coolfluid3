// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Signal.hpp"

#include "Mesh/CMesh.hpp"

#include "Solver/Actions/Proto/NodeLooper.hpp"

#include "FieldGeneration.hpp"


using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver::Actions::Proto;

using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace FieldGeneration {

////////////////////////////////////////////////////////////////////////////////

CF::Common::ComponentBuilder < FieldGenerator, Common::Component, LibFieldGeneration > aFieldGenerator_Builder;

void create_constant_scalar_field(CMesh& mesh, const std::string& field_name, const std::string& var_name, const Real value)
{
  MeshTerm<1, ScalarField> heat(field_name, var_name);
  mesh.create_scalar_field(field_name, var_name, CField::Basis::POINT_BASED);
  for_each_node(mesh.topology(), heat = value);
}

FieldGenerator::FieldGenerator(const std::string& name) : Component(name)
{
  OptionURI::Ptr mesh_path = boost::dynamic_pointer_cast<OptionURI>( m_options.add_option<OptionURI>("Mesh", "Mesh to add the field to", std::string()) );
  mesh_path->supported_protocol(CF::Common::URI::Scheme::CPATH);
  mesh_path->mark_basic();

  m_options.add_option< OptionT<std::string> >("FieldName", "Name of the field to create", "")->mark_basic();
  m_options.add_option< OptionT<std::string> >("VariableName", "Name of the variable to create", "")->mark_basic();
  m_options.add_option< OptionT<Real> >("Value", "Value for every node in the field", 0.)->mark_basic();

  this->regist_signal("update" , "Update the field value according to the options", "Update")->signal->connect( boost::bind ( &FieldGenerator::signal_update, this, _1 ) );
}

void FieldGenerator::signal_update(Common::SignalArgs& node)
{
  Component::Ptr mesh_component = access_component_ptr(URI(option("Mesh").value_str()));
  if(!mesh_component)
    throw ValueNotFound(FromHere(), "The path for the mesh is incorrect");

  CMesh::Ptr mesh = boost::dynamic_pointer_cast<CMesh>(mesh_component);
  if(!mesh)
    throw BadValue(FromHere(), "The given path does not point to a mesh");

  const std::string field_name = option("FieldName").value_str();
  const std::string var_name = option("VariableName").value_str();
  const Real value = option("Value").value<Real>();

  // Get the field, if it exists
  CField::Ptr field = boost::dynamic_pointer_cast<CField>(mesh->get_child_ptr(field_name));

  // We can update the field if it exists AND contains the variable that we need
  if(field && !field->has_variable(var_name))
  {
    throw ValueExists(FromHere(), "A field with name " + field_name + " already exists, but it does not contain a variable named " + var_name);
  }

  // If the field didn't exist, we create it
  if(!field)
  {
    mesh->create_scalar_field(field_name, var_name, CF::Mesh::CField::Basis::POINT_BASED);
  }

  // Proto placeholder
  MeshTerm<0, ScalarField> s(field_name, var_name);

  // Update the field value
  for_each_node(mesh->topology(), s = value);
}

////////////////////////////////////////////////////////////////////////////////

} // FieldGeneration
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////
