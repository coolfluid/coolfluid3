// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::LagrangeSF"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/DynTable.hpp"
#include "common/List.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/ContinuousDictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"

#include "mesh/Integrators/Gauss.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::mesh::Integrators;
using namespace cf3::common;

//////////////////////////////////////////////////////////////////////////////

struct Nodes_Fixture
{
  /// common setup for each test case
  Nodes_Fixture()
  {
     mesh2d = Core::instance().root().create_component<Mesh>  ( "mesh2d" );
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

    // Read the a .neu mesh as 2D mixed mesh
    boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

    // Read the mesh
    meshreader->read_mesh_into("../../resources/quadtriag.neu",*mesh2d);
  }

  /// common tear-down for each test case
  ~Nodes_Fixture()
  {
  }
  /// common values accessed by all tests goes here
  Handle<Mesh> mesh2d;

  Elements& get_first_region()
  {
    BOOST_FOREACH(Elements& region, find_components_recursively<Elements>(*mesh2d))
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
  const Elements& firstRegion = get_first_region();
  const Table<Real>& coords = firstRegion.geometry_fields().coordinates();
  const Table<Uint>& conn = firstRegion.geometry_space().connectivity();
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
  const Elements& firstRegion = get_first_region();
  const Table<Real>& coords = firstRegion.geometry_fields().coordinates();
  const Table<Uint>& conn = firstRegion.geometry_space().connectivity();
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
  boost::shared_ptr<Dictionary> geometry = allocate_component<ContinuousDictionary>("geometry_fieds");
  BOOST_CHECK( is_not_null(geometry) );

  Handle<Field> coords = geometry->create_component<Field>("coordinates");
  coords->set_row_size(2u);
  coords->create_descriptor("coords[vec]",2u);

  // Tagging this component will cache it to geometry->coordinates()
  coords->add_tag(mesh::Tags::coordinates());


  geometry->resize(10);
  BOOST_CHECK_EQUAL(geometry->coordinates().size() , 10u);
  BOOST_CHECK_EQUAL(geometry->coordinates().row_size() , 2u);
  BOOST_CHECK_EQUAL(geometry->rank().size() , 10u);
  BOOST_CHECK_EQUAL(geometry->glb_idx().size() , 10u);

  // created on demand
  BOOST_CHECK_EQUAL(geometry->glb_elem_connectivity().size() , 10u);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

