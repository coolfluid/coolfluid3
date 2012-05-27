// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh octtree"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "common/Core.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/FindComponents.hpp"
#include "common/Link.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"
#include "common/Table.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/MeshGenerator.hpp"
#include "mesh/Octtree.hpp"
#include "mesh/StencilComputerOcttree.hpp"
#include "mesh/MeshWriter.hpp"

using namespace boost;
using namespace boost::assign;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct Octtree_Fixture
{
  /// common setup for each test case
  Octtree_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     m_argc = boost::unit_test::framework::master_test_suite().argc;
     m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~Octtree_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here

  int m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Octtree_TestSuite, Octtree_Fixture )


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  PE::Comm::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Octtree_creation )
{
  // create meshreader
  boost::shared_ptr< MeshGenerator > mesh_generator = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","mesh_generator");
  Core::instance().root().add_component(mesh_generator);
  mesh_generator->options().set("mesh",Core::instance().root().uri()/"mesh");
  mesh_generator->options().set("lengths",std::vector<Real>(2,10.));
  mesh_generator->options().set("nb_cells",std::vector<Uint>(2,5));
  mesh_generator->options().set("part",0u);
  mesh_generator->options().set("nb_parts",1u);

  Mesh& mesh = mesh_generator->generate();
  Handle<Dictionary> dict = mesh.geometry_fields().handle<Dictionary>();
  Octtree& octtree = *mesh.create_component<Octtree>("octtree");

  // Create and configure octtree.
  octtree.options().set("nb_elems_per_cell", 1u );
  octtree.options().set("mesh", mesh.handle<Mesh>());

  // Following configuration option has priority over the the previous one.
  std::vector<Uint> nb_cells = boost::assign::list_of(5)(5);
  octtree.options().set("nb_cells", nb_cells );

  BOOST_CHECK(true);

  Entity element;
  RealVector2 coord;

  coord << 1. , 1. ;
  element = octtree.find_element(coord);
  BOOST_CHECK_EQUAL(element.idx,0u);

  coord << 3. , 1. ;
  element = octtree.find_element(coord);
  BOOST_CHECK_EQUAL(element.idx,1u);

  coord << 1 , 3. ;
  element = octtree.find_element(coord);
  BOOST_CHECK_EQUAL(element.idx,5u);


  Handle<StencilComputerOcttree> stencil_computer = Core::instance().root().create_component<StencilComputerOcttree>("stencilcomputer");  
  stencil_computer->options().set("dict", dict );

  SpaceElem space_elem = SpaceElem(mesh.elements()[0]->space(*dict),7);
  std::vector<SpaceElem> stencil;
  stencil_computer->options().set("stencil_size", 1u );
  stencil_computer->compute_stencil(space_elem, stencil);
  BOOST_CHECK_EQUAL(stencil.size(), 1u);

  stencil_computer->options().set("stencil_size", 2u );
  stencil_computer->compute_stencil(space_elem, stencil);
  BOOST_CHECK_EQUAL(stencil.size(), 9u);

  stencil_computer->options().set("stencil_size", 10u );
  stencil_computer->compute_stencil(space_elem, stencil);
  BOOST_CHECK_EQUAL(stencil.size(), 20u);

  stencil_computer->options().set("stencil_size", 21u );
  stencil_computer->compute_stencil(space_elem, stencil);
  BOOST_CHECK_EQUAL(stencil.size(), 25u); // mesh size

  CFinfo << stencil_computer->tree() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Octtree_parallel )
{
  Handle< MeshGenerator > mesh_generator(Core::instance().root().get_child("mesh_generator"));
  mesh_generator->options().set("mesh",Core::instance().root().uri()/"parallel_mesh");
  mesh_generator->options().set("lengths",std::vector<Real>(2,10.));
  mesh_generator->options().set("nb_cells",std::vector<Uint>(2,5));
  mesh_generator->options().set("part",PE::Comm::instance().rank());
  mesh_generator->options().set("nb_parts",PE::Comm::instance().size());
  Mesh& mesh = mesh_generator->generate();

  Octtree& octtree = *mesh.create_component<Octtree>("octtree");

  // Create and configure interpolator.
  octtree.options().set("nb_elems_per_cell", 1u );
  octtree.options().set("mesh", mesh.handle<Mesh>() );
  octtree.create_octtree();

  // Following configuration option has priority over the the previous one.
  std::vector<Uint> nb_cells = boost::assign::list_of(5)(5);
  octtree.options().set("nb_cells", nb_cells );

  boost::multi_array<Real,2> coordinates;
  coordinates.resize(boost::extents[2][2]);
  coordinates[0][XX] = 5.;  coordinates[0][YY] = 2.5;
  coordinates[1][XX] = 5.;  coordinates[1][YY] = 7.5;

  std::vector<Uint> ranks;
  octtree.find_cell_ranks(coordinates,ranks);

  BOOST_CHECK_EQUAL(ranks[0] , 0u);
  BOOST_CHECK_EQUAL(ranks[1] , 1u);


//  MeshWriter& gmsh_writer = mesh.create_component("gmsh_writer","cf3.mesh.gmsh.Writer").as_type<MeshWriter>();
//  gmsh_writer.write_from_to(mesh,"octtree.msh");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize )
{
  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

