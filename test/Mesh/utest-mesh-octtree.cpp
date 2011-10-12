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

#include "Common/Core.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"

#include "Common/FindComponents.hpp"
#include "Common/CLink.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CMeshGenerator.hpp"
#include "Mesh/COcttree.hpp"
#include "Mesh/CStencilComputerOcttree.hpp"

using namespace boost;
using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct Octtree_Fixture
{
  /// common setup for each test case
  Octtree_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~Octtree_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Octtree_TestSuite, Octtree_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Octtree_creation )
{
  COcttree::Ptr octtree = allocate_component<COcttree>("octtree");
  BOOST_CHECK_EQUAL(octtree->name(),"octtree");

  BOOST_CHECK( true );

  // create meshreader
  CMeshGenerator::Ptr mesh_generator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","mesh_generator");
  Core::instance().root().add_component(mesh_generator);
  mesh_generator->configure_option("mesh",Core::instance().root().uri()/"mesh");
  mesh_generator->configure_option("lengths",std::vector<Real>(2,10.));
  mesh_generator->configure_option("nb_cells",std::vector<Uint>(2,5));
  CMesh& mesh = mesh_generator->generate();

  // Create and configure interpolator.
  octtree->configure_option("nb_elems_per_cell", (Uint) 1 );
  octtree->configure_option("mesh", mesh.uri() );
  // Following configuration option has priority over the the previous one.
  std::vector<Uint> nb_cells = boost::assign::list_of(5)(5);
  octtree->configure_option("nb_cells", nb_cells );

  BOOST_CHECK(true);

  octtree->create_octtree();

  CElements::ConstPtr elements;
  Uint idx(0);
  RealVector2 coord;

  coord << 1. , 1. ;
  boost::tie(elements,idx) = octtree->find_element(coord);
  BOOST_CHECK_EQUAL(idx,0u);

  coord << 3. , 1. ;
  boost::tie(elements,idx) = octtree->find_element(coord);
  BOOST_CHECK_EQUAL(idx,1u);

  coord << 1 , 3. ;
  boost::tie(elements,idx) = octtree->find_element(coord);
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

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

