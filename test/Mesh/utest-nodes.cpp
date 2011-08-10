// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::LagrangeSF"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/FindComponents.hpp"


#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/Geometry.hpp"

#include "Mesh/Integrators/Gauss.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Common;

//////////////////////////////////////////////////////////////////////////////

struct Nodes_Fixture
{
  /// common setup for each test case
  Nodes_Fixture()
  {
     mesh2d = Core::instance().root().create_component_ptr<CMesh>  ( "mesh2d" );
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

    // Read the a .neu mesh as 2D mixed mesh
    CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");

    // Read the mesh
    meshreader->read_mesh_into("quadtriag.neu",*mesh2d);
  }

  /// common tear-down for each test case
  ~Nodes_Fixture()
  {
  }
  /// common values accessed by all tests goes here
  boost::shared_ptr<CMesh> mesh2d;

  CElements& get_first_region()
  {
    BOOST_FOREACH(CElements& region, find_components_recursively<CElements>(*mesh2d))
    {
        return (region);
    }
    throw ShouldNotBeHere(FromHere(), "");
  }

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Nodes, Nodes_Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( FillVector )
{
  const CElements& firstRegion = get_first_region();
  const CTable<Real>& coords = firstRegion.geometry().coordinates();
  const CTable<Uint>& conn = firstRegion.node_connectivity();
  const Uint element_count = conn.size();
  std::vector<RealVector> node_vector(conn.row_size(), RealVector(coords.row_size()));
  for(Uint element = 0; element != element_count; ++element)
  {
    fill(node_vector, coords, conn[element]);
    for(Uint node_idx = 0; node_idx != conn.row_size(); ++node_idx)
    {
      for(Uint xyz = 0; xyz != coords.row_size(); ++xyz)
      {
        BOOST_CHECK_EQUAL(node_vector[node_idx][xyz], coords[conn[element][node_idx]][xyz]);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE( FillMatrix )
{
  const CElements& firstRegion = get_first_region();
  const CTable<Real>& coords = firstRegion.geometry().coordinates();
  const CTable<Uint>& conn = firstRegion.node_connectivity();
  const Uint element_count = conn.size();
  RealMatrix node_matrix(conn.row_size(), coords.row_size());
  for(Uint element = 0; element != element_count; ++element)
  {
    fill(node_matrix, coords, conn[element]);
    for(Uint node_idx = 0; node_idx != conn.row_size(); ++node_idx)
    {
      for(Uint xyz = 0; xyz != coords.row_size(); ++xyz)
      {
        BOOST_CHECK_EQUAL(node_matrix(node_idx, xyz), coords[conn[element][node_idx]][xyz]);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Construct_Geometry )
{
  Geometry::Ptr nodes = allocate_component<Geometry>("nodes");
  BOOST_CHECK( is_not_null(nodes) );

  nodes->resize(10);
  BOOST_CHECK_EQUAL(nodes->coordinates().size() , 10u);
  BOOST_CHECK_EQUAL(nodes->rank().size() , 10u);
  BOOST_CHECK_EQUAL(nodes->glb_idx().size() , 10u);
  BOOST_CHECK_EQUAL(nodes->glb_elem_connectivity().size() , 0u);



}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

