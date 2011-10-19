// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests Mesh::Actions::CGlobalConnectivity"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/PE/debug.hpp"
#include "Common/PE/Comm.hpp"

#include "Mesh/Actions/Interpolate.hpp"

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::Common::PE;

////////////////////////////////////////////////////////////////////////////////

struct TestCGlobalConnectivity_Fixture
{
  /// common setup for each test case
  TestCGlobalConnectivity_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestCGlobalConnectivity_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  int m_argc;
  char** m_argv;

  /// common values accessed by all tests goes here
  static CMesh::Ptr mesh;
};

CMesh::Ptr TestCGlobalConnectivity_Fixture::mesh = Core::instance().root().create_component_ptr<CMesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestCGlobalConnectivity_TestSuite, TestCGlobalConnectivity_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Init )
{
  Core::instance().initiate(m_argc,m_argv);
  PE::Comm::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( build )
{
  Core::instance().initiate(m_argc,m_argv);

  CMeshGenerator::Ptr mesh_generator = Core::instance().root().create_component_ptr<CSimpleMeshGenerator>("mesh_generator");
  mesh_generator->configure_option("mesh",Core::instance().root().uri()/"rect");

  mesh_generator->configure_option("lengths",std::vector<Real>(2,10.));
  std::vector<Real> offsets(2);
  offsets[XX] = 0.;
  offsets[YY] = -0.5;
  mesh_generator->configure_option("offsets",offsets);
  std::vector<Uint> nb_cells(2);
  nb_cells[XX] = 10u;
  nb_cells[YY] = 1u;
  mesh_generator->configure_option("nb_cells",nb_cells);
  CMesh& rect = mesh_generator->generate();

  mesh_generator->configure_option("mesh",Core::instance().root().uri()/"line");
  mesh_generator->configure_option("nb_cells",std::vector<Uint>(1,20));
  mesh_generator->configure_option("lengths",std::vector<Real>(1,20.));
//  mesh_generator->configure_option("offsets",std::vector<Real>(1,-5));

  CMesh& line = mesh_generator->generate();

  Field& source = rect.geometry().create_field("solution");
  Field& target = line.geometry().create_field("solution");


  for(Uint i=0; i<source.size();++i)
  {
    source[i][0] = source.coordinates()[i][XX];
  }

  Interpolate& interpolator = Core::instance().root().create_component<Interpolate>("interpolator");
  interpolator.configure_option("source",source.uri());
  interpolator.configure_option("target",target.uri());
  interpolator.execute();


  CMeshWriter& gmsh_writer = Core::instance().root().create_component("gmsh_writer","CF.Mesh.Gmsh.CWriter").as_type<CMeshWriter>();
  gmsh_writer.configure_option("fields",std::vector<URI>(1,target.uri()) );
  gmsh_writer.write_from_to(line,"line-interpolated.msh");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Terminate )
{
  PE::Comm::instance().finalize();
  Core::instance().terminate();
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

