// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::BoundingBox"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "mesh/BoundingBox.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshGenerator.hpp"
#include "mesh/Field.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct boundingboxMPITests_Fixture
{
  /// common setup for each test case
  boundingboxMPITests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~boundingboxMPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( boundingboxMPITests_TestSuite, boundingboxMPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  Core::instance().initiate(m_argc,m_argv);
  PE::Comm::instance().init(m_argc,m_argv);
  Core::instance().environment().options().configure_option("log_level",3u);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test )
{
  Uint dim=2;

  // Generate a mesh
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("mesh");
  boost::shared_ptr< MeshGenerator > generate_mesh = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","meshgenerator");
  generate_mesh->options().configure_option("nb_cells",std::vector<Uint>(dim,10));
  generate_mesh->options().configure_option("lengths",std::vector<Real>(dim,10.));
  generate_mesh->options().configure_option("mesh",mesh->uri());
  generate_mesh->execute();

  // Build a bounding box
  Handle<BoundingBox> bounding_box = mesh->create_component<BoundingBox>("bounding_box");
  BOOST_CHECK_NO_THROW(bounding_box->build(*mesh));
  
  // Check if bounding box is correct for each rank
  switch (PE::Comm::instance().rank())
  {
    case 0:
      BOOST_CHECK_EQUAL(bounding_box->min()[XX] , 0.);
      BOOST_CHECK_EQUAL(bounding_box->min()[YY] , 0.);
      BOOST_CHECK_EQUAL(bounding_box->max()[XX] , 10.);
      BOOST_CHECK_EQUAL(bounding_box->max()[YY] , 5.);
      break;
      
    case 1:
      BOOST_CHECK_EQUAL(bounding_box->min()[XX] , 0.);
      BOOST_CHECK_EQUAL(bounding_box->min()[YY] , 5.);
      BOOST_CHECK_EQUAL(bounding_box->max()[XX] , 10.);
      BOOST_CHECK_EQUAL(bounding_box->max()[YY] , 10.);
      break;
  }
  
  // Make global bounding box --> all ranks have same bounding_box
  bounding_box->make_global();
  BOOST_CHECK_EQUAL(bounding_box->min()[XX] , 0.);
  BOOST_CHECK_EQUAL(bounding_box->min()[YY] , 0.);
  BOOST_CHECK_EQUAL(bounding_box->max()[XX] , 10.);
  BOOST_CHECK_EQUAL(bounding_box->max()[YY] , 10.);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

