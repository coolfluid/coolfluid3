// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests cf3::mesh::UnifiedData<T>"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/UnifiedData.hpp"
#include "mesh/NodeElementConnectivity.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct UnifiedData_Fixture
{
  /// common setup for each test case
  UnifiedData_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~UnifiedData_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( UnifiedData_TestSuite, UnifiedData_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors)
{
  boost::shared_ptr<UnifiedData> unified_elems = allocate_component<UnifiedData>("unified_elems");
  BOOST_CHECK_EQUAL(unified_elems->name(),"unified_elems");
  BOOST_CHECK_EQUAL(UnifiedData::type_name(), "UnifiedData");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( data_location )
{
  // create meshreader
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  BOOST_CHECK( true );

  Mesh& mesh = *Core::instance().root().create_component<Mesh>("mesh");
  meshreader->read_mesh_into("../../resources/quadtriag.neu",mesh);

  BOOST_CHECK( true );

  boost::shared_ptr<UnifiedData> unified_elems = allocate_component<UnifiedData>("unified_elems");


  boost_foreach(Elements& elements, find_components_recursively<Elements>(mesh))
    unified_elems->add(elements);





  Handle< Component > elements;
  Uint elem_idx;

  BOOST_CHECK_EQUAL( unified_elems->size() , 28u );
  tie(elements,elem_idx) = unified_elems->location(25);

  for (Uint i=0; i<unified_elems->size(); ++i)
  {
    tie(elements,elem_idx) = unified_elems->location(i);
    CFinfo << i << ": " << elements->uri().path() << "    ["<<elem_idx<<"]" << CFendl;
  }

  boost::shared_ptr<UnifiedData> unified_nodes = allocate_component<UnifiedData>("unified_nodes");
  boost_foreach(Dictionary& nodes, find_components_recursively<Dictionary>(mesh))
    unified_nodes->add(nodes);

  Handle< Component > nodes;
  Uint node_idx;

  BOOST_CHECK_EQUAL( unified_nodes->size() , 16u );

  CFinfo << CFendl;
  for (Uint i=0; i<unified_nodes->size(); ++i)
  {
    tie(nodes,node_idx) = unified_nodes->location(i);
    CFinfo << i << ": " << nodes->uri().path() << "    ["<<node_idx<<"]" << CFendl;
  }

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

