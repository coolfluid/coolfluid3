// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests cf3::Mesh::CNodeElementConnectivity"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/CRoot.hpp"
#include "common/FindComponents.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CConnectivity.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::Mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct NodeElementConnectivity_Fixture
{
  /// common setup for each test case
  NodeElementConnectivity_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~NodeElementConnectivity_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( NodeElementConnectivity_TestSuite, NodeElementConnectivity_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors)
{
  CNodeElementConnectivity::Ptr c = allocate_component<CNodeElementConnectivity>("nodes_to_elements");
  BOOST_CHECK_EQUAL(c->name(),"nodes_to_elements");
  BOOST_CHECK_EQUAL(CNodeElementConnectivity::type_name(), "CNodeElementConnectivity");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( node_elem_connectivity )
{
  // create meshreader
  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");

  CMesh& mesh = Core::instance().root().create_component<CMesh>("quadtriag");
  meshreader->read_mesh_into("quadtriag.neu",mesh);

  BOOST_CHECK( true );

  // create and setup node to elements connectivity
  CNodeElementConnectivity::Ptr c = mesh.create_component_ptr<CNodeElementConnectivity>("node_elem_connectivity");
  c->setup( find_component<CRegion>(mesh) );

  BOOST_CHECK( true );

  // Output whole node to elements connectivity
  CFinfo << c->connectivity() << CFendl;

  // Output connectivity of node 10
  CDynTable<Uint>::ConstRow elements = c->connectivity()[10];
  CFinfo << CFendl << "node 10 is connected to elements: \n";
  boost_foreach(const Uint elem, elements)
  {
    Component::Ptr connected_comp;
    Uint connected_idx;
    tie(connected_comp,connected_idx) = c->elements().location(elem);
    CFinfo << "   " << connected_comp->uri().path() << "  [" <<connected_idx <<  "] " << CFendl;
  }
  CFinfo << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

