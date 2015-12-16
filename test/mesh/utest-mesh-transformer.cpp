
// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh reading"

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

#include "common/OptionT.hpp"
#include "common/Log.hpp"
#include "common/Group.hpp"
#include "common/FindComponents.hpp"


#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "common/Table.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshTransformer.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct MeshTransformer_Fixture
{
  /// common setup for each test case
  MeshTransformer_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;

    root = allocate_component<Group>("Root");
    reader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","MyReader");
    domain = root->create_component<Domain>("MyDom");

    root->add_component( reader );

    ExceptionManager::instance().ExceptionOutputs = false;
    ExceptionManager::instance().ExceptionDumps = false;
  }

  /// common tear-down for each test case
  ~MeshTransformer_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  boost::shared_ptr< Component > root;
  boost::shared_ptr< MeshReader > reader;
  Handle< Domain > domain;

  static Handle< Mesh > mesh;
  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

Handle< Mesh > MeshTransformer_Fixture::mesh;

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshTransformer_TestSuite, MeshTransformer_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_mesh )
{
  // boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  //
  // // the file to read from
  // boost::filesystem::path fp_in ("../../resources/quadtriag.neu");
  //
  // // the mesh to store in
  //
  // meshreader->do_read_mesh_into(fp_in,mesh);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_arguments )
{
  boost::shared_ptr< MeshTransformer > transformer = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.Info","info");

  std::vector<std::string> args;
  args.push_back("variable:bool=true");
  args.push_back("array_vars:array<bool>=true,false,true");
  //args.push_back("center:double=[x,y,z]");

  //transformer->configure(args);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

