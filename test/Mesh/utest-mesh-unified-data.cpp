// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests CF::Mesh::CUnifiedData<T>"

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CNodes.hpp"
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
  CUnifiedData<CElements>::Ptr unified_elems = allocate_component<CUnifiedData<CElements> >("unified_elems");
  BOOST_CHECK_EQUAL(unified_elems->name(),"unified_elems");
  BOOST_CHECK_EQUAL(CUnifiedData<CElements>::type_name(), "CUnifiedData<CElements>");
  BOOST_CHECK_EQUAL(CUnifiedData<CNodes>::type_name(), "CUnifiedData<CNodes>");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( data_location )
{
  // create meshreader
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");

  BOOST_CHECK( true );

  boost::filesystem::path fp_source ("quadtriag.neu");
  CMesh::Ptr mesh = meshreader->create_mesh_from(fp_source);

  BOOST_CHECK( true );

  CUnifiedData<CElements>::Ptr unified_elems = allocate_component<CUnifiedData<CElements> >("unified_elems");



  unified_elems->add_data(find_components_recursively<CElements>(*mesh).as_vector());
  




  CElements::Ptr elements;
  CElements::ConstPtr const_elements;
  Uint elem_idx;
  
  BOOST_CHECK_EQUAL( unified_elems->size() , 28u );
  tie(elements,elem_idx) = unified_elems->data_location(25);

  for (Uint i=0; i<unified_elems->size(); ++i)
  {
    tie(elements,elem_idx) = unified_elems->data_location(i);
    CFinfo << i << ": " << elements->full_path().path() << "    ["<<elem_idx<<"]" << CFendl;
  }
  
  CUnifiedData<CNodes>::Ptr unified_nodes = allocate_component<CUnifiedData<CNodes> >("unified_nodes");
  unified_nodes->add_data(find_components_recursively<CNodes>(*mesh).as_vector());
  
  CNodes::Ptr nodes;
  Uint node_idx;
  
  BOOST_CHECK_EQUAL( unified_nodes->size() , 16u );

  CFinfo << CFendl;
  for (Uint i=0; i<unified_nodes->size(); ++i)
  {
    tie(nodes,node_idx) = unified_nodes->data_location(i);
    CFinfo << i << ": " << nodes->full_path().path() << "    ["<<node_idx<<"]" << CFendl;
  }

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

