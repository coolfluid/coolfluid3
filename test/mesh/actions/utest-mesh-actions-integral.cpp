// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh::actions::SurfaceIntegral"

#include <boost/test/unit_test.hpp>

#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/PE/Comm.hpp"

#include "mesh/actions/CreateField.hpp"
#include "mesh/actions/SurfaceIntegral.hpp"
#include "mesh/actions/VolumeIntegral.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/SimpleMeshGenerator.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;

////////////////////////////////////////////////////////////////////////////////

struct TestFixture
{
  /// common setup for each test case
  TestFixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestFixture()
  {
  }

  /// possibly common functions used on the tests below

  int m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestSuite, TestFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Initiate )
{
 Core::instance().initiate(m_argc,m_argv);
 PE::Comm::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( IntegrateOnSquare )
{
  // Create a P1 square mesh with side 10.
  Handle<MeshGenerator> mesh_generator = Core::instance().root().create_component<SimpleMeshGenerator>("generate_square");
  mesh_generator->options().set("mesh",Core::instance().root().uri()/"mesh");
  mesh_generator->options().set("lengths",std::vector<Real>(2,10.));
  mesh_generator->options().set("nb_cells",std::vector<Uint>(2,5));
  Mesh& mesh = mesh_generator->generate();

  // Create a field with value 1. everywhere
  boost::shared_ptr<CreateField> create_field = allocate_component<CreateField>("create_field");
  std::vector<std::string> functions;
  functions.push_back("f=1.");
  create_field->options().set("functions",functions);
  create_field->options().set("name",std::string("field"));
  create_field->options().set("dict",mesh.geometry_fields().uri());
  create_field->transform(mesh);
  Field& field = *mesh.geometry_fields().get_child("field")->handle<Field>();
  
  // Surface Integral of the field. It should return the total line length = 4*10.
  boost::shared_ptr<SurfaceIntegral> surface_integration = allocate_component<SurfaceIntegral>("surface_integration");
  surface_integration->options().set("order",1u);
  Real surface_integral = surface_integration->integrate(field, std::vector< Handle<Region> >(1, mesh.topology().handle<Region>()) );
  BOOST_CHECK_EQUAL(surface_integral,40.);

  // Volume Integral of the field. It should return the total area = 10*10.
  boost::shared_ptr<VolumeIntegral> volume_integration = allocate_component<VolumeIntegral>("volume_integration");
  volume_integration->options().set("order",1u);
  Real volume_integral = volume_integration->integrate(field, std::vector< Handle<Region> >(1, mesh.topology().handle<Region>()) );
  BOOST_CHECK_EQUAL(volume_integral,100.);
}

BOOST_AUTO_TEST_CASE( IntegrateOnCube )
{
  // Create a P1 square mesh with side 10.
  Handle<MeshGenerator> mesh_generator = Core::instance().root().create_component<SimpleMeshGenerator>("generate_cube");
  mesh_generator->options().set("mesh",Core::instance().root().uri()/"cube");
  mesh_generator->options().set("lengths",std::vector<Real>(3,10.));
  mesh_generator->options().set("nb_cells",std::vector<Uint>(3,5));
  Mesh& mesh = mesh_generator->generate();

  // Create a field with value 1. everywhere
  boost::shared_ptr<CreateField> create_field = allocate_component<CreateField>("create_field");
  std::vector<std::string> functions;
  functions.push_back("f=1.");
  create_field->options().set("functions",functions);
  create_field->options().set("name",std::string("field"));
  create_field->options().set("dict",mesh.geometry_fields().uri());
  create_field->transform(mesh);
  Field& field = *mesh.geometry_fields().get_child("field")->handle<Field>();
  
  // Surface Integral of the field. It should return the total line length = 6*10*10.
  boost::shared_ptr<SurfaceIntegral> surface_integration = allocate_component<SurfaceIntegral>("surface_integration");
  surface_integration->options().set("order",1u);
  // Real surface_integral = surface_integration->integrate(field, std::vector< Handle<Region> >(1, mesh.topology().handle<Region>()) );
  // BOOST_CHECK_EQUAL(surface_integral,600.);

  // Volume Integral of the field. It should return the total area = 10*10*10.
  boost::shared_ptr<VolumeIntegral> volume_integration = allocate_component<VolumeIntegral>("volume_integration");
  volume_integration->options().set("order",1u);
  Real volume_integral = volume_integration->integrate(field, std::vector< Handle<Region> >(1, mesh.topology().handle<Region>()) );
  BOOST_CHECK_EQUAL(volume_integral,1000.);
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

