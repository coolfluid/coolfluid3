// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::FieldManager"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Environment.hpp"
#include "common/Core.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Field.hpp"
#include "mesh/SimpleMeshGenerator.hpp"
#include "mesh/Space.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Dictionary.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;
using namespace cf3::math;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( FieldManagerSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_FieldManager )
{
  Core::instance().environment().options().set("exception_aborts",false);
  Core::instance().environment().options().set("exception_backtrace",false);
  Core::instance().environment().options().set("exception_outputs",false);
  Component& root = Core::instance().root();

  // tag to use (normally supplied by the solver)
  const std::string tag = "solution";

  Handle<VariableManager> var_manager = root.create_component<VariableManager>("varmanager");
  var_manager->create_descriptor(tag, "a, b[v], c[t]").options().set(common::Tags::dimension(), 2u);

  // Test mesh
  Mesh& mesh = *root.create_component<Mesh>("mesh");
  Tools::MeshGeneration::create_rectangle(mesh, 1., 1., 10, 10);

  // FieldManager
  FieldManager& field_manager = *root.create_component<FieldManager>("fieldmanager");
  field_manager.options().set("variable_manager", var_manager);

  // Do this twice, to ensure the second run does nothing
  field_manager.create_field(tag, mesh.geometry_fields());
  field_manager.create_field(tag, mesh.geometry_fields());

  BOOST_CHECK(is_not_null(mesh.geometry_fields().get_child(tag)));
  Field& field = mesh.geometry_fields().field(tag);
  BOOST_CHECK(field.has_variable("a"));
  BOOST_CHECK(field.row_size() == 7);

  // Now change the descriptor and ensure there is an error
  var_manager->get_child(tag)->remove_tag(tag);
  var_manager->create_descriptor(tag, "a, b[v], c[t]").options().set(common::Tags::dimension(), 3u);
  BOOST_CHECK_THROW(field_manager.create_field(tag, mesh.geometry_fields()), SetupError);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

