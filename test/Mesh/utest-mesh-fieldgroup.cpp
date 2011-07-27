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
  CSimpleMeshGenerator::create_rectangle(*m_mesh,5.,5.,5u,5u);
//  Core::instance().root().add_component(m_mesh);
}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_FieldGroup )
{
  CMesh& mesh = *m_mesh;

  BOOST_CHECK_NO_THROW(mesh.nodes().check_sanity());

  // ----------------------------------------------------------------------------------------------
  // CHECK element-based field group building


  FieldGroup& elem_fields = mesh.create_field_group("elems_P0", FieldGroup::Basis::ELEMENT_BASED, CEntities::MeshSpaces::to_str(CEntities::MeshSpaces::MESH_ELEMENTS));
  FieldGroup& cell_fields = mesh.create_field_group("cells_P0", FieldGroup::Basis::CELL_BASED,    CEntities::MeshSpaces::to_str(CEntities::MeshSpaces::MESH_ELEMENTS));
  FieldGroup& face_fields = mesh.create_field_group("faces_P0", FieldGroup::Basis::FACE_BASED,    CEntities::MeshSpaces::to_str(CEntities::MeshSpaces::MESH_ELEMENTS));

  BOOST_CHECK_EQUAL( elem_fields.size() , 45);
  BOOST_CHECK_EQUAL( cell_fields.size() , 25);
  BOOST_CHECK_EQUAL( face_fields.size() , 20);

  // ----------------------------------------------------------------------------------------------
  // CHECK field building inside field groups

  Field& solution = elem_fields.create_field("solution","rho[1],V[2],p[1]");
  Field& volume   = cell_fields.create_field("volume");

  BOOST_CHECK_EQUAL(solution.size() , elem_fields.size());
  BOOST_CHECK_EQUAL(solution.row_size() , 4);
  BOOST_CHECK_EQUAL(solution.array().num_elements() , solution.size()*solution.row_size());
  BOOST_CHECK_EQUAL(solution.field_group().uri().string() , elem_fields.uri().string() );
  BOOST_CHECK_EQUAL(volume.size() , cell_fields.size());
  BOOST_CHECK_EQUAL(volume.field_group().uri().string() , cell_fields.uri().string() );

  BOOST_CHECK_EQUAL(cell_fields.fields().size() , 1u);
  boost_foreach(Field& field, cell_fields.fields())
    BOOST_CHECK_EQUAL( field.field_group().uri().string() , cell_fields.uri().string());

  BOOST_CHECK_EQUAL(cell_fields.field("volume").uri().string() , volume.uri().string());

  // ----------------------------------------------------------------------------------------------
  // CHECK P1 point-based field group building

  // Create space for Lagrange P1 elements
  boost_foreach(CEntities& elements, find_components_recursively<CEntities>(mesh.topology()))
    elements.create_space("P1","CF.Mesh.SF.SF"+elements.element_type().shape_name()+"LagrangeP1");

  // Create field group for the space "P1"
  FieldGroup& point_P1_fields = mesh.create_field_group("points_P1", FieldGroup::Basis::POINT_BASED, "P1");

  BOOST_CHECK_EQUAL ( point_P1_fields.size() , mesh.nodes().size() );

  // ----------------------------------------------------------------------------------------------
  // CHECK P2 point-based field group building

  // Create space for Lagrange P2 elements
  boost_foreach(CEntities& elements, find_components_recursively<CEntities>(mesh.topology()))
    elements.create_space("P2","CF.Mesh.SF.SF"+elements.element_type().shape_name()+"LagrangeP2");

  // Create field group for the space "P2"
  FieldGroup& point_P2_fields = mesh.create_field_group("points_P2", FieldGroup::Basis::POINT_BASED, "P2");
  BOOST_CHECK_EQUAL ( point_P2_fields.size() , 121u );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

