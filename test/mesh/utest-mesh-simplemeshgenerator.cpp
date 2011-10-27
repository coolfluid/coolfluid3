// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::neu::Reader"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Root.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshGenerator.hpp"
#include "mesh/MeshTransformer.hpp"

#include "common/DynTable.hpp"
#include "common/List.hpp"
#include "common/Table.hpp"
#include "mesh/FieldGroup.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct SimpleMeshGeneratorTests_Fixture
{
  /// common setup for each test case
  SimpleMeshGeneratorTests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~SimpleMeshGeneratorTests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( SimpleMeshGeneratorTests_TestSuite, SimpleMeshGeneratorTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  PE::Comm::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( generate_1d_mesh )
{

  MeshGenerator::Ptr meshgenerator = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","1Dgenerator");

  meshgenerator->configure_option("mesh",URI("//Root/line"));
  meshgenerator->configure_option("nb_cells",std::vector<Uint>(1,10));
  meshgenerator->configure_option("lengths",std::vector<Real>(1,10.));
  Mesh& mesh = meshgenerator->generate();


  CFinfo << "elements count = " << mesh.topology().recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << mesh.topology().recursive_nodes_count() << CFendl;

  Uint nb_ghosts=0;

  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,"line.msh");

  BOOST_CHECK(true);

  CFinfo << mesh.tree() << CFendl;

  FieldGroup& nodes = mesh.geometry_fields();
  for (Uint n=0; n<nodes.size(); ++n)
  {
    if (nodes.is_ghost(n))
    {
      CFinfo << "node " << n << " is a ghost node" << CFendl;
      ++nb_ghosts;
    }
  }
  CFinfo << "ghost node count = " << nb_ghosts << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( generate_2d_mesh )
{

  MeshGenerator::Ptr meshgenerator = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","1Dgenerator");

  meshgenerator->configure_option("mesh",URI("//Root/rect"));
  meshgenerator->configure_option("nb_cells",std::vector<Uint>(2,2));
  meshgenerator->configure_option("lengths",std::vector<Real>(2,2.));
  Mesh& mesh = meshgenerator->generate();


  CFinfo << "elements count = " << mesh.topology().recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << mesh.topology().recursive_nodes_count() << CFendl;

  Uint nb_ghosts=0;

  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,"rect.msh");

  BOOST_CHECK(true);

  CFinfo << mesh.tree() << CFendl;

  FieldGroup& nodes = mesh.geometry_fields();
  for (Uint n=0; n<nodes.size(); ++n)
  {
    if (nodes.is_ghost(n))
    {
      CFinfo << "node " << n << " is a ghost node" << CFendl;
      ++nb_ghosts;
    }
  }
  CFinfo << "ghost node count = " << nb_ghosts << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

