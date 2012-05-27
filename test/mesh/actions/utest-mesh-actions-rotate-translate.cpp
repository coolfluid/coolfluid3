// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh::actions::Rotate"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/PE/debug.hpp"
#include "common/PE/Comm.hpp"

#include "math/Consts.hpp"

#include "mesh/actions/Rotate.hpp"

#include "mesh/MeshWriter.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/SimpleMeshGenerator.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;
using namespace cf3::common::PE;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

struct TestRotate_Fixture
{
  /// common setup for each test case
  TestRotate_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestRotate_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  int m_argc;
  char** m_argv;

  /// common values accessed by all tests goes here
  static Handle< Mesh > mesh;
};

Handle< Mesh > TestRotate_Fixture::mesh = Core::instance().root().create_component<Mesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestRotate_TestSuite, TestRotate_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Init )
{
  Core::instance().initiate(m_argc,m_argv);
  //PE::Comm::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_rotate_2d )
{
  Handle<MeshGenerator> mesh_generator = Core::instance().root().create_component<SimpleMeshGenerator>("mesh_generator_rect");
  mesh_generator->options().set("mesh",Core::instance().root().uri()/"rect");

  mesh_generator->options().set("lengths",std::vector<Real>(2,10.));
  std::vector<Uint> nb_cells = list_of(10)(5);
  mesh_generator->options().set("nb_cells",nb_cells);
  Mesh& mesh = mesh_generator->generate();

  boost::shared_ptr<MeshTransformer> rotate = boost::dynamic_pointer_cast<MeshTransformer>(build_component("cf3.mesh.actions.Rotate","rotate"));
  std::vector<Real> axis_point = list_of(5.)(5.);
  rotate->options().set("axis_point",axis_point);
  rotate->options().set("angle",math::Consts::pi()/2.);
  rotate->transform(mesh);
  mesh.write_mesh("file:rotated_rect.msh");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_rotate_3d )
{
  Handle<MeshGenerator> mesh_generator = Core::instance().root().create_component<SimpleMeshGenerator>("mesh_generator_box");
  mesh_generator->options().set("mesh",Core::instance().root().uri()/"box");

  mesh_generator->options().set("lengths",std::vector<Real>(3,10.));
  std::vector<Uint> nb_cells = list_of(10)(5)(2);
  mesh_generator->options().set("nb_cells",nb_cells);
  Mesh& mesh = mesh_generator->generate();

  boost::shared_ptr<MeshTransformer> rotate = boost::dynamic_pointer_cast<MeshTransformer>(build_component("cf3.mesh.actions.Rotate","rotate"));
  std::vector<Real> axis_direction = list_of(1)(1)(1);
  std::vector<Real> axis_point = list_of(5.)(5.)(5.);
  rotate->options().set("axis_direction",axis_direction);
  rotate->options().set("axis_point",axis_point);
  rotate->options().set("angle",math::Consts::pi()/2.);
  rotate->transform(mesh);
  mesh.write_mesh("file:rotated_box.msh");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_translate_2d )
{

  Mesh& mesh = *Core::instance().root().get_child("rect")->handle<Mesh>();

  boost::shared_ptr<MeshTransformer> translate = boost::dynamic_pointer_cast<MeshTransformer>(build_component("cf3.mesh.actions.Translate","translate"));
  std::vector<Real> tvec = list_of(-5)(-5);
  translate->options().set("vector",tvec);
  translate->transform(mesh);

  mesh.write_mesh("file:translated_rect.msh");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_translate_3d )
{

  Mesh& mesh = *Core::instance().root().get_child("box")->handle<Mesh>();

  boost::shared_ptr<MeshTransformer> translate = boost::dynamic_pointer_cast<MeshTransformer>(build_component("cf3.mesh.actions.Translate","translate"));
  std::vector<Real> tvec = list_of(-5)(-5)(-5);
  translate->options().set("vector",tvec);
  translate->transform(mesh);

  mesh.write_mesh("file:translated_box.msh");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Terminate )
{
  //PE::Comm::instance().finalize();
  Core::instance().terminate();
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

