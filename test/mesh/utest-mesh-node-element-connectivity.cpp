// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests cf3::mesh::NodeElementConnectivity"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/NodeElementConnectivity.hpp"
#include "mesh/Connectivity.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
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
  boost::shared_ptr<NodeElementConnectivity> c = allocate_component<NodeElementConnectivity>("nodes_to_elements");
  BOOST_CHECK_EQUAL(c->name(),"nodes_to_elements");
  BOOST_CHECK_EQUAL(NodeElementConnectivity::type_name(), "NodeElementConnectivity");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( node_elem_connectivity )
{
  // create meshreader
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  Mesh& mesh = *Core::instance().root().create_component<Mesh>("quadtriag");
  meshreader->read_mesh_into("../../resources/quadtriag.neu",mesh);

  BOOST_CHECK( true );

  // create and setup node to elements connectivity
  Handle<NodeElementConnectivity> c = mesh.create_component<NodeElementConnectivity>("node_elem_connectivity");
  c->setup( find_component<Region>(mesh) );

  BOOST_CHECK( true );

  // Output whole node to elements connectivity
  CFinfo << c->connectivity() << CFendl;

  // Output connectivity of node 10
  DynTable<Uint>::ConstRow elements = c->connectivity()[10];
  CFinfo << CFendl << "node 10 is connected to elements: \n";
  boost_foreach(const Uint elem, elements)
  {
    Handle< Component > connected_comp;
    Uint connected_idx;
    tie(connected_comp,connected_idx) = c->elements().location(elem);
    CFinfo << "   " << connected_comp->uri().path() << "  [" <<connected_idx <<  "] " << CFendl;
  }
  CFinfo << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

