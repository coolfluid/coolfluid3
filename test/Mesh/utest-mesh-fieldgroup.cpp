// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::FieldGroup"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/FieldGroup.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"
#include "Mesh/CNodes.hpp"

using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

struct FieldGroupTests_Fixture
{
  /// common setup for each test case
  FieldGroupTests_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~FieldGroupTests_Fixture()
  {
  }

  /// common mesh accessed by all tests
  static CMesh::Ptr m_mesh;
};

CMesh::Ptr FieldGroupTests_Fixture::m_mesh = Core::instance().root().create_component_ptr<CMesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( FieldGroupTests_TestSuite, FieldGroupTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_MeshCreation )
{
  CSimpleMeshGenerator::create_rectangle(*m_mesh,10.,10.,10u,10u);
//  Core::instance().root().add_component(m_mesh);
}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_FieldGroup )
{
  CMesh& mesh = *m_mesh;

  FieldGroup& fields = mesh.create_component<FieldGroup>("mesh_fields");

  fields.configure_option("space",std::string("default"));
  fields.configure_option("type", FieldGroup::Basis::to_str(FieldGroup::Basis::ELEMENT_BASED));
  fields.configure_option("topology",mesh.topology().uri());

  CFinfo << mesh.tree() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

