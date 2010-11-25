// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::CField"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Log.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

struct FieldTests_Fixture
{
  /// common setup for each test case
  FieldTests_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

    CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");

    // the file to read from
    boost::filesystem::path fp_in ("quadtriag.neu");
    // the mesh to store in
    m_mesh = meshreader->create_mesh_from(fp_in);

  }

  /// common tear-down for each test case
  ~FieldTests_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here

  CMesh::Ptr m_mesh;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( FieldTests_TestSuite, FieldTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( FieldTest )
{
  CMesh& mesh = *m_mesh;

  mesh.create_field("Volume",1,CField::ELEMENT_BASED);
	std::vector<std::string> solution_vars = list_of("rho[1]")("V[3]")("p[1]");
  mesh.create_field("Solution",solution_vars,CField::NODE_BASED);

	CFinfo << mesh.tree() << CFendl;
	
  // Check if the fields have been created inside the mesh
  BOOST_CHECK_EQUAL(mesh.field("Volume").full_path().string(),"mesh/Volume");
  BOOST_CHECK_EQUAL(mesh.field("Solution").full_path().string(),"mesh/Solution");

  // Check if support is filled in correctly
  BOOST_CHECK_EQUAL(mesh.field("Volume").support().name(), "quadtriag");
  BOOST_CHECK_EQUAL(mesh.field("Volume").support().recursive_filtered_elements_count(IsElementsVolume()), (Uint) 16);
  BOOST_CHECK_EQUAL(mesh.field("Volume").subfield("gas").support().recursive_elements_count(), (Uint) 6);

  // Check if connectivity_table is properly linked to the support ones
  BOOST_CHECK_EQUAL(mesh.field("Volume").subfield("gas").elements("elements_Quad2DLagrangeP1").connectivity_table().size(), (Uint) 2);
  BOOST_CHECK_EQUAL(&mesh.field("Volume").subfield("gas").elements("elements_Quad2DLagrangeP1").connectivity_table(),
                    &mesh.domain().subregion("gas").elements("elements_Quad2DLagrangeP1").connectivity_table());

  // test the CRegion::get_field function, to return the matching field
  BOOST_CHECK_EQUAL(mesh.domain().get_field("Volume").full_path().string(),"mesh/Volume");
  BOOST_CHECK_EQUAL(mesh.domain().subregion("gas").get_field("Volume").full_path().string(),"mesh/Volume/gas");

  BOOST_CHECK_EQUAL(mesh.look_component("quadtriag/gas")->full_path().string(),"mesh/quadtriag/gas");
  BOOST_CHECK_EQUAL(mesh.look_component("quadtriag/gas/../liquid")->full_path().string(),"mesh/quadtriag/liquid");
  BOOST_CHECK_EQUAL(mesh.look_component_type<CRegion>("quadtriag/gas/../liquid")->get_field("Volume").full_path().string(),"mesh/Volume/liquid");


  // Check if element based data is correctly created
  BOOST_CHECK_EQUAL(mesh.look_component_type<CFieldElements>("Volume/gas/elements_Quad2DLagrangeP1")->data().size(), (Uint) 2);
  BOOST_CHECK_EQUAL(mesh.look_component_type<CFieldElements>("Volume/gas/elements_Quad2DLagrangeP1")->data().row_size(), (Uint) 1);

  // Check if node based data is correctly created
  BOOST_CHECK_EQUAL(mesh.look_component_type<CFieldElements>("Solution/gas/elements_Quad2DLagrangeP1")->data().row_size(), (Uint) 5);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

