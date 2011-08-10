// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::FieldManager"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

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

using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Math;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( FieldManagerSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_FieldManager )
{
  CRoot& root = Core::instance().root();

  // tag to use (normally supplied by the solver)
  const std::string tag = "solution";

  VariableManager& var_manager = root.create_component<VariableManager>("varmanager");
  var_manager.create_descriptor(tag, "a, b[v], c[t]").configure_option("dimensions", 2u);

  // Test mesh
  CMesh& mesh = root.create_component<CMesh>("mesh");

  // FieldManager
  FieldManager& field_manager = root.create_component<FieldManager>("fieldmanager");
  field_manager.configure_option("variable_manager", var_manager.uri());

  field_manager.create_fields(tag, mesh, FieldGroup::Basis::POINT_BASED);

  BOOST_CHECK(is_not_null(mesh.get_child_ptr(tag)));
  Field& field = mesh.get_child(tag).as_type<Field>();
  BOOST_CHECK(field.has_variable("a"));
  BOOST_CHECK(field.data().row_size() == 7);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

