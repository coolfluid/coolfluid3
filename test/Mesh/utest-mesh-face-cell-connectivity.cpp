// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests CF::Mesh::CFaceCellConnectivity"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/ConnectivityData.hpp"

using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Tools;

////////////////////////////////////////////////////////////////////////////////

struct FaceCellConnectivity_Fixture //: public Testing::TimedTestFixture
{
  /// common setup for each test case
  FaceCellConnectivity_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

  }

  /// common tear-down for each test case
  ~FaceCellConnectivity_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  static CMesh::Ptr m_mesh;
};

CMesh::Ptr FaceCellConnectivity_Fixture::m_mesh;
////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( FaceCellConnectivity_TestSuite, FaceCellConnectivity_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors )
{
  CFaceCellConnectivity::Ptr c = allocate_component<CFaceCellConnectivity>("faces_to_cells");
  BOOST_CHECK_EQUAL(c->name(),"faces_to_cells");
  BOOST_CHECK_EQUAL(CFaceCellConnectivity::type_name(), "CFaceCellConnectivity");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( create_mesh )
{
     
 // create meshreader
 // CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
 // boost::filesystem::path fp_source ("quadtriag.neu");
 // m_mesh = meshreader->create_mesh_from(fp_source);
 
  m_mesh = allocate_component<CMesh>("mesh");
  Uint scale = 2;
  MeshGeneration::create_rectangle(*m_mesh, 4., 2., scale*2u, scale*2u);
  BOOST_CHECK(true);
}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( face_elem_connectivity )
{

  // create and setup node to elements connectivity  
  CFaceCellConnectivity::Ptr c = m_mesh->create_component<CFaceCellConnectivity>("face_cell_connectivity");
  c->setup( find_component<CRegion>(*m_mesh) );

  BOOST_CHECK_EQUAL(c->connectivity().size() , 40u);
  
  CFinfo << "nodes of face 0 : ";
  boost_foreach(const Uint node, c->nodes(0) )
    CFinfo << "  " << node;
  CFinfo << CFendl;
  // Output whole node to elements connectivity
  //CFinfo << c->connectivity() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

