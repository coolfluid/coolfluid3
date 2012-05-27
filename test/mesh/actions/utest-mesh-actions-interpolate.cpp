// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh::actions::GlobalConnectivity"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/PE/debug.hpp"
#include "common/PE/Comm.hpp"

#include "mesh/actions/Interpolate.hpp"

#include "mesh/MeshWriter.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/SimpleMeshGenerator.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;
using namespace cf3::common::PE;

////////////////////////////////////////////////////////////////////////////////

struct TestGlobalConnectivity_Fixture
{
  /// common setup for each test case
  TestGlobalConnectivity_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestGlobalConnectivity_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  int m_argc;
  char** m_argv;

  /// common values accessed by all tests goes here
  static Handle< Mesh > mesh;
};

Handle< Mesh > TestGlobalConnectivity_Fixture::mesh = Core::instance().root().create_component<Mesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestGlobalConnectivity_TestSuite, TestGlobalConnectivity_Fixture )

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

  Handle<MeshGenerator> mesh_generator = Core::instance().root().create_component<SimpleMeshGenerator>("mesh_generator");
  mesh_generator->options().set("mesh",Core::instance().root().uri()/"rect");

  mesh_generator->options().set("lengths",std::vector<Real>(2,10.));
  std::vector<Real> offsets(2);
  offsets[XX] = 0.;
  offsets[YY] = -0.5;
  mesh_generator->options().set("offsets",offsets);
  std::vector<Uint> nb_cells(2);
  nb_cells[XX] = 10u;
  nb_cells[YY] = 1u;
  mesh_generator->options().set("nb_cells",nb_cells);
  Mesh& rect = mesh_generator->generate();

  mesh_generator->options().set("mesh",Core::instance().root().uri()/"line");
  mesh_generator->options().set("nb_cells",std::vector<Uint>(1,20));
  mesh_generator->options().set("lengths",std::vector<Real>(1,20.));
//  mesh_generator->options().set("offsets",std::vector<Real>(1,-5));

  Mesh& line = mesh_generator->generate();

  Field& source = rect.geometry_fields().create_field("solution");
  Field& target = line.geometry_fields().create_field("solution");


  for(Uint i=0; i<source.size();++i)
  {
    source[i][0] = source.coordinates()[i][XX];
  }

  Interpolate& interpolator = *Core::instance().root().create_component<Interpolate>("interpolator");
  interpolator.options().set("source",source.handle<Field const>());
  interpolator.options().set("target",target.handle<Field>());
  interpolator.execute();


  Handle<MeshWriter> gmsh_writer(Core::instance().root().create_component("gmsh_writer","cf3.mesh.gmsh.Writer"));
  gmsh_writer->options().set("fields",std::vector<URI>(1,target.uri()) );
  gmsh_writer->write_from_to(line,"line-interpolated.msh");
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

