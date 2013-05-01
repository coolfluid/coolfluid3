// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh::actions::FieldCreation"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/PE/debug.hpp"
#include "common/PE/Comm.hpp"

#include "math/Consts.hpp"

#include "mesh/actions/CreateField.hpp"
#include "mesh/actions/ComputeFieldGradient.hpp"
#include "mesh/actions/Rotate.hpp"

#include "mesh/MeshWriter.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/SimpleMeshGenerator.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;
using namespace cf3::common::PE;

////////////////////////////////////////////////////////////////////////////////

struct TestFieldCreation_Fixture
{
  /// common setup for each test case
  TestFieldCreation_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestFieldCreation_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  int m_argc;
  char** m_argv;

  /// common values accessed by all tests goes here
  static Handle< Mesh > mesh;
};

Handle< Mesh > TestFieldCreation_Fixture::mesh = Core::instance().root().create_component<Mesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestFieldCreation_TestSuite, TestFieldCreation_Fixture )

////////////////////////////////////////////////////////////////////////////////

//BOOST_AUTO_TEST_CASE( Init )
//{
//  Core::instance().initiate(m_argc,m_argv);
//  PE::Comm::instance().init(m_argc,m_argv);
//}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( execute )
{
  Core::instance().initiate(m_argc,m_argv);

  Handle<MeshGenerator> mesh_generator = Core::instance().root().create_component<SimpleMeshGenerator>("mesh_generator");
  mesh_generator->options().set("mesh",Core::instance().root().uri()/"rect");
  mesh_generator->options().set("lengths",std::vector<Real>(2,2*math::Consts::pi()));
  mesh_generator->options().set("nb_cells",std::vector<Uint>(2,10));
  Mesh& rect = mesh_generator->generate();

  boost::shared_ptr<Rotate> rotate_mesh = allocate_component<Rotate>("rotate_mesh");
  rotate_mesh->options().set("mesh",rect.handle());
  rotate_mesh->options().set("angle",math::Consts::pi()/4.);
  rotate_mesh->options().set("axis_point",std::vector<Real>(2,math::Consts::pi()));
  rotate_mesh->execute();

  boost::shared_ptr<CreateField> create_field = allocate_component<CreateField>("create_field");
  std::vector<std::string> functions;
  functions.push_back("f=cos(x)+cos(y)");
  functions.push_back("U[2]=[x,y]");
  create_field->options().set("functions",functions);
  create_field->options().set("name",std::string("field"));
  create_field->options().set("dict",rect.geometry_fields().uri());
  create_field->transform(rect);

  Field& field = *rect.geometry_fields().get_child("field")->handle<Field>();
  Field& grad = rect.geometry_fields().create_field("grad","dfdx,dudx,dvdx,dfdy,dudy,dvdy");

  boost::shared_ptr<ComputeFieldGradient> compute_gradient = allocate_component<ComputeFieldGradient>("compute_gradient");

  compute_gradient->options().set("mesh",rect.handle());
  compute_gradient->options().set("field",field.handle());
  compute_gradient->options().set("field_gradient",grad.handle());

  compute_gradient->execute();

  std::vector<URI> fields;
  fields.push_back(field.uri());
  fields.push_back(grad.uri());
  rect.write_mesh("file:out-utest-mesh-actions-fieldcreation.msh",fields);
}

////////////////////////////////////////////////////////////////////////////////

//BOOST_AUTO_TEST_CASE( Terminate )
//{
//  PE::Comm::instance().finalize();
//  Core::instance().terminate();
//}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

