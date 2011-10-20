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
#include "common/FindComponents.hpp"
#include "common/CLink.hpp"
#include "common/CRoot.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CMeshGenerator.hpp"
#include "Mesh/COcttree.hpp"
#include "Mesh/CStencilComputerOcttree.hpp"
#include "Mesh/CMeshWriter.hpp"

using namespace boost;
using namespace boost::assign;
using namespace cf3;
using namespace cf3::Mesh;
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
  CMeshGenerator::Ptr mesh_generator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","mesh_generator");
  Core::instance().root().add_component(mesh_generator);
  mesh_generator->configure_option("mesh",Core::instance().root().uri()/"mesh");
  mesh_generator->configure_option("lengths",std::vector<Real>(2,10.));
  mesh_generator->configure_option("nb_cells",std::vector<Uint>(2,5));
  mesh_generator->configure_option("part",0u);
  mesh_generator->configure_option("nb_parts",1u);

  CMesh& mesh = mesh_generator->generate();

  COcttree& octtree = mesh.create_component<COcttree>("octtree");

  // Create and configure interpolator.
  octtree.configure_option("nb_elems_per_cell", 1u );
  octtree.configure_option("mesh", mesh.uri() );

  // Following configuration option has priority over the the previous one.
  std::vector<Uint> nb_cells = boost::assign::list_of(5)(5);
  octtree.configure_option("nb_cells", nb_cells );

  BOOST_CHECK(true);

  CElements::ConstPtr elements;
  Uint idx(0);
  RealVector2 coord;

  coord << 1. , 1. ;
  boost::tie(elements,idx) = octtree.find_element(coord);
  BOOST_CHECK_EQUAL(idx,0u);

  coord << 3. , 1. ;
  boost::tie(elements,idx) = octtree.find_element(coord);
  BOOST_CHECK_EQUAL(idx,1u);

  coord << 1 , 3. ;
  boost::tie(elements,idx) = octtree.find_element(coord);
  BOOST_CHECK_EQUAL(idx,5u);


  CStencilComputerOcttree::Ptr stencil_computer = Core::instance().root().create_component_ptr<CStencilComputerOcttree>("stencilcomputer");
  stencil_computer->configure_option("mesh", find_component<CMesh>(Core::instance().root()).uri() );

  std::vector<Uint> stencil;
  stencil_computer->configure_option("stencil_size", 1u );
  stencil_computer->compute_stencil(7, stencil);
  BOOST_CHECK_EQUAL(stencil.size(), 1u);

  stencil_computer->configure_option("stencil_size", 2u );
  stencil_computer->compute_stencil(7, stencil);
  BOOST_CHECK_EQUAL(stencil.size(), 9u);

  stencil_computer->configure_option("stencil_size", 10u );
  stencil_computer->compute_stencil(7, stencil);
  BOOST_CHECK_EQUAL(stencil.size(), 20u);

  stencil_computer->configure_option("stencil_size", 21u );
  stencil_computer->compute_stencil(7, stencil);
  BOOST_CHECK_EQUAL(stencil.size(), 25u); // mesh size

  CFinfo << stencil_computer->tree() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Octtree_parallel )
{
  CMeshGenerator::Ptr mesh_generator = Core::instance().root().get_child("mesh_generator").as_ptr<CMeshGenerator>();
  mesh_generator->configure_option("mesh",Core::instance().root().uri()/"parallel_mesh");
  mesh_generator->configure_option("lengths",std::vector<Real>(2,10.));
  mesh_generator->configure_option("nb_cells",std::vector<Uint>(2,5));
  mesh_generator->configure_option("part",PE::Comm::instance().rank());
  mesh_generator->configure_option("nb_parts",PE::Comm::instance().size());
  CMesh& mesh = mesh_generator->generate();

  COcttree& octtree = mesh.create_component<COcttree>("octtree");

  // Create and configure interpolator.
  octtree.configure_option("nb_elems_per_cell", 1u );
  octtree.configure_option("mesh", mesh.uri() );
  octtree.create_octtree();

  // Following configuration option has priority over the the previous one.
  std::vector<Uint> nb_cells = boost::assign::list_of(5)(5);
  octtree.configure_option("nb_cells", nb_cells );

  boost::multi_array<Real,2> coordinates;
  coordinates.resize(boost::extents[2][2]);
  coordinates[0][XX] = 5.;  coordinates[0][YY] = 2.5;
  coordinates[1][XX] = 5.;  coordinates[1][YY] = 7.5;

  std::vector<Uint> ranks;
  octtree.find_cell_ranks(coordinates,ranks);

  BOOST_CHECK_EQUAL(ranks[0] , 0u);
  BOOST_CHECK_EQUAL(ranks[1] , 1u);


//  CMeshWriter& gmsh_writer = mesh.create_component("gmsh_writer","CF.Mesh.Gmsh.CWriter").as_type<CMeshWriter>();
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

