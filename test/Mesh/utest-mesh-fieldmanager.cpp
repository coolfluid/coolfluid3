// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::Mesh::FieldManager"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "common/Log.hpp"
#include "common/CEnv.hpp"
#include "common/Core.hpp"
#include "common/CRoot.hpp"

#include "Math/VariableManager.hpp"
#include "Math/VariablesDescriptor.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/FieldManager.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/Geometry.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::Mesh;
using namespace cf3::common;
using namespace cf3::Math;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( FieldManagerSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_FieldManager )
{
  Core::instance().environment().configure_option("exception_aborts",false);
  Core::instance().environment().configure_option("exception_backtrace",false);
  Core::instance().environment().configure_option("exception_outputs",false);
  CRoot& root = Core::instance().root();

  // tag to use (normally supplied by the solver)
  const std::string tag = "solution";

  VariableManager& var_manager = root.create_component<VariableManager>("varmanager");
  var_manager.create_descriptor(tag, "a, b[v], c[t]").configure_option(common::Tags::dimension(), 2u);

  // Test mesh
  CMesh& mesh = root.create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_rectangle(mesh, 1., 1., 10, 10);

  // FieldManager
  FieldManager& field_manager = root.create_component<FieldManager>("fieldmanager");
  field_manager.configure_option("variable_manager", var_manager.uri());

  // Do this twice, to ensure the second run does nothing
  field_manager.create_field(tag, mesh.geometry());
  field_manager.create_field(tag, mesh.geometry());

  BOOST_CHECK(is_not_null(mesh.geometry().get_child_ptr(tag)));
  Field& field = mesh.geometry().field(tag);
  BOOST_CHECK(field.has_variable("a"));
  BOOST_CHECK(field.row_size() == 7);
  
  // Now change the descriptor and ensure there is an error
  var_manager.get_child(tag).remove_tag(tag);
  var_manager.create_descriptor(tag, "a, b[v], c[t]").configure_option(common::Tags::dimension(), 3u);
  BOOST_CHECK_THROW(field_manager.create_field(tag, mesh.geometry()), SetupError);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

