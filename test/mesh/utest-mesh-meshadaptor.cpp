// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Mesh Manipulations"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Core.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshGenerator.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/MeshAdaptor.hpp"

#include "common/DynTable.hpp"
#include "common/List.hpp"
#include "common/Table.hpp"
#include "mesh/Dictionary.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct MeshManipulationsTests_Fixture
{
  /// common setup for each test case
  MeshManipulationsTests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~MeshManipulationsTests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshManipulationsTests_TestSuite, MeshManipulationsTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  PE::Comm::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_manipulations_general_use )
{
  // Generate a simple 1D line-mesh of 10 cells
  boost::shared_ptr< MeshGenerator > meshgenerator = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","1Dgenerator");
  meshgenerator->options().set("mesh",URI("//line2"));
  meshgenerator->options().set("nb_cells",std::vector<Uint>(1,10));
  meshgenerator->options().set("lengths",std::vector<Real>(1,10.));
  Mesh& mesh = meshgenerator->generate();

  // Create a MeshAdaptor object to manipulate the elements
  MeshAdaptor mesh_adaptor(mesh);

  // Allocations (not important)
  PE::Buffer buf;
  PackedElement unpacked_elem(mesh);
  PackedNode    unpacked_node(mesh);

  // Prepare the mesh-adaptor, rebuilding element-node connectivity tables to become global
  BOOST_CHECK_NO_THROW(  mesh_adaptor.prepare()  );

  // Pack an element and a node
  PackedElement elem(mesh, /* entities_idx= */ 0, /* loc_elem_idx = */ 0);
  PackedNode    node(mesh, /* dictionary_idx= */ 0, /* loc_node_idx = */ 0);

  // Load the element and node into a buffer
  buf << elem << node;

  // Remove the element and node from the mesh (not applied until finish() is called)
  BOOST_CHECK_NO_THROW(  mesh_adaptor.remove_element(elem)  );
  BOOST_CHECK_NO_THROW(  mesh_adaptor.remove_node(node) );

  // Unload the same element and node from the buffer
  buf >> unpacked_elem >> unpacked_node;

  // Add the same element and node back into the mesh (not applied until finish() is called)
  BOOST_CHECK_NO_THROW(  mesh_adaptor.add_element(unpacked_elem)  );
  BOOST_CHECK_NO_THROW(  mesh_adaptor.add_node(unpacked_node) );

  // Finish the mesh-adaptor, applying all changes (in this case the changes cancel out),
  // and restore the element-node connectivity tables to become local
  BOOST_CHECK_NO_THROW(  mesh_adaptor.finish()  );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_move_elements )
{
  // Generate a simple 1D line-mesh of 10 cells
  boost::shared_ptr< MeshGenerator > meshgenerator = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","1Dgenerator");
  meshgenerator->options().set("mesh",URI("//line3"));
  meshgenerator->options().set("nb_cells",std::vector<Uint>(1,10));
  meshgenerator->options().set("lengths",std::vector<Real>(1,10.));
  Mesh& mesh = meshgenerator->generate();

  // Create a MeshAdaptor object to manipulate the elements
  MeshAdaptor mesh_adaptor(mesh);

  // Allocations (not important)
  PE::Buffer buf;
  PackedElement unpacked_elem(mesh);
  PackedNode    unpacked_node(mesh);

  // Prepare the mesh-adaptor, rebuilding element-node connectivity tables to become global


  BOOST_CHECK_EQUAL(mesh.elements().size(), 3u);

  std::vector< std::vector<std::vector<Uint> > > change_set(PE::Comm::instance().size(),
                                                            std::vector<std::vector<Uint> >(mesh.elements().size()));

  if (PE::Comm::instance().size() >= 2)
  {

    switch (PE::Comm::instance().rank())
    {
    case 0:
//    BOOST_CHECK(change_set.size() == PE::Comm::instance().size());
      change_set[1][0].push_back(4);
      break;
    case 1:
      change_set[0][0].push_back(4);
      break;
  }
}
  BOOST_CHECK(true);

  BOOST_CHECK_NO_THROW(  mesh_adaptor.prepare()  );

  BOOST_CHECK_NO_THROW(mesh_adaptor.move_elements(change_set));

  // Finish the mesh-adaptor, applying all changes (in this case the changes cancel out),
  // and restore the element-node connectivity tables to become local
  BOOST_CHECK_NO_THROW(  mesh_adaptor.finish()  );
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_element_node_connectivity_rebuilding )
{

  // Generate a simple 1D line-mesh of 10 cells
  boost::shared_ptr< MeshGenerator > meshgenerator = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","1Dgenerator");
  meshgenerator->options().set("mesh",URI("//line"));
  meshgenerator->options().set("nb_cells",std::vector<Uint>(1,10));
  meshgenerator->options().set("lengths",std::vector<Real>(1,10.));
  Mesh& mesh = meshgenerator->generate();
  mesh.geometry_fields().rebuild_map_glb_to_loc();

  // Create a MeshAdaptor object to manipulate the elements
  MeshAdaptor mesh_adaptor(mesh);
  mesh_adaptor.create_element_buffers();

  // Remove an element
  mesh_adaptor.remove_element(0,1);
  mesh_adaptor.flush_elements();

  if (PE::Comm::instance().size() == 2)
  {
    BOOST_CHECK_EQUAL(mesh.elements()[0]->size(),4u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[0][0], 0u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[0][1], 1u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[1][0], 4u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[1][1], 5u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[2][0], 2u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[2][1], 3u);
  }

  // Make all element-node connectivities use global node indices
  BOOST_CHECK_NO_THROW(mesh_adaptor.make_element_node_connectivity_global());

  if (PE::Comm::instance().size() == 2)
  {
    switch (PE::Comm::instance().rank())
    {
    case 0:
      BOOST_CHECK_EQUAL(mesh.elements()[0]->size(),4u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[0][0], 0u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[0][1], 1u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[1][0], 4u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[1][1], 5u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[2][0], 2u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[2][1], 3u);
      break;
    case 1:
      BOOST_CHECK_EQUAL(mesh.elements()[0]->size(),4u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[0][0], 5u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[0][1], 6u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[1][0], 9u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[1][1], 10u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[2][0], 7u);
      BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[2][1], 8u);
      break;
    }
  }

  BOOST_CHECK(true);

  // entities_idx: index as it appears in mesh.elements() vector; can be found through mesh.find_elements_idx()
  const Uint entities_idx = mesh.access_component_checked("topology/interior/Line")->handle<Entities>()->entities_idx();
  BOOST_CHECK(true);
  BOOST_CHECK_EQUAL(entities_idx , 0u);

  const Uint elem_idx = 1u; // index local to entities component
  BOOST_CHECK(true);

  PackedElement packed_elem(mesh,entities_idx,elem_idx);

  BOOST_CHECK(true);

  // dict_idx: index as it appears in mesh.dictionaries() vector; can be found through mesh.find_dictionary_idx()
  const Uint dict_idx = 0u;//mesh.find_dictionary_idx(mesh.geometry_fields().handle<Dictionary>() );
  const Uint node_idx = 2u;
  PackedNode packed_node(mesh,dict_idx,node_idx);

  BOOST_CHECK(true);
  PE::Buffer buf;

  buf << packed_elem;
  buf << packed_node;

  BOOST_CHECK(true);

  PackedElement unpacked_elem(mesh);
  PackedNode    unpacked_node(mesh);

  buf >> unpacked_elem;
  buf >> unpacked_node;

  BOOST_CHECK(true);

  BOOST_CHECK_EQUAL(unpacked_node.loc_idx(),   packed_node.loc_idx());
  BOOST_CHECK_EQUAL(unpacked_node.glb_idx(),   packed_node.glb_idx());
  BOOST_CHECK_EQUAL(unpacked_node.rank(),      packed_node.rank());
  BOOST_CHECK(unpacked_node.field_values() ==  packed_node.field_values());

  // Restore the node-connectivity to normal
  BOOST_CHECK_NO_THROW(mesh_adaptor.restore_element_node_connectivity());

  if (PE::Comm::instance().size() == 2)
  {
    BOOST_CHECK_EQUAL(mesh.elements()[0]->size(),4u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[0][0], 0u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[0][1], 1u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[1][0], 4u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[1][1], 5u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[2][0], 2u);
    BOOST_CHECK_EQUAL(mesh.elements()[0]->geometry_space().connectivity()[2][1], 3u);
  }

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

