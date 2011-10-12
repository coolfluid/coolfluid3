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
#include "Mesh/CStencilComputerRings.hpp"

using namespace boost;
using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct StencilComputerRings_Fixture
{
  /// common setup for each test case
  StencilComputerRings_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~StencilComputerRings_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( StencilComputerRings_TestSuite, StencilComputerRings_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( StencilComputerRings_creation )
{

  // create meshreader
  CMeshGenerator::Ptr mesh_generator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","mesh_generator");
  Core::instance().root().add_component(mesh_generator);
  mesh_generator->configure_option("mesh",Core::instance().root().uri()/"mesh");
  mesh_generator->configure_option("lengths",std::vector<Real>(2,10.));
  mesh_generator->configure_option("nb_cells",std::vector<Uint>(2,5));
  CMesh& mesh = mesh_generator->generate();

  CStencilComputerRings::Ptr stencil_computer = Core::instance().root().create_component_ptr<CStencilComputerRings>("stencilcomputer");
  stencil_computer->configure_option("mesh", mesh.uri() );

  std::vector<Uint> stencil;
//  stencil_computer->configure_option("stencil_size", 10u );
  stencil_computer->configure_option("nb_rings", 1u );
  stencil_computer->compute_stencil(7, stencil);
  BOOST_CHECK_EQUAL(stencil.size(), 9u);

  stencil_computer->configure_option("nb_rings", 2u );
  stencil_computer->compute_stencil(7, stencil);
  BOOST_CHECK_EQUAL(stencil.size(), 20u);

  stencil_computer->configure_option("nb_rings", 3u );
  stencil_computer->compute_stencil(7, stencil);
  BOOST_CHECK_EQUAL(stencil.size(), 25u);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

