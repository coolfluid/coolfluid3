// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests CF::Mesh::CUnifiedData<T>"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CUnifiedData.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"

using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

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
  CUnifiedData::Ptr unified_elems = allocate_component<CUnifiedData>("unified_elems");
  BOOST_CHECK_EQUAL(unified_elems->name(),"unified_elems");
  BOOST_CHECK_EQUAL(CUnifiedData::type_name(), "CUnifiedData");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( data_location )
{
  // create meshreader
  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");

  BOOST_CHECK( true );

  CMesh& mesh = Core::instance().root().create_component<CMesh>("mesh");
  meshreader->read_mesh_into("quadtriag.neu",mesh);

  BOOST_CHECK( true );

  CUnifiedData::Ptr unified_elems = allocate_component<CUnifiedData>("unified_elems");


  boost_foreach(CElements& elements, find_components_recursively<CElements>(mesh))
    unified_elems->add(elements);





  Component::Ptr elements;
  Uint elem_idx;

  BOOST_CHECK_EQUAL( unified_elems->size() , 28u );
  tie(elements,elem_idx) = unified_elems->location(25);

  for (Uint i=0; i<unified_elems->size(); ++i)
  {
    tie(elements,elem_idx) = unified_elems->location(i);
    CFinfo << i << ": " << elements->uri().path() << "    ["<<elem_idx<<"]" << CFendl;
  }

  CUnifiedData::Ptr unified_nodes = allocate_component<CUnifiedData>("unified_nodes");
  boost_foreach(Geometry& nodes, find_components_recursively<Geometry>(mesh))
    unified_nodes->add(nodes);

  Component::Ptr nodes;
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

