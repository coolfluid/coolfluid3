// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::Neu::CReader"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshGenerator.hpp"
#include "Mesh/CMeshTransformer.hpp"

#include "Mesh/CDynTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/Geometry.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

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
  MPI::PE::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( generate_1d_mesh )
{

  CMeshGenerator::Ptr meshgenerator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","1Dgenerator");

  meshgenerator->configure_option("parent",URI("//Root"));
  meshgenerator->configure_option("name",std::string("line"));
  meshgenerator->configure_option("nb_cells",std::vector<Uint>(1,10));
  meshgenerator->configure_option("lengths",std::vector<Real>(1,10.));
  meshgenerator->execute();
  CMesh& mesh = Core::instance().root().get_child("line").as_type<CMesh>();



  CFinfo << "elements count = " << mesh.topology().recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << mesh.topology().recursive_nodes_count() << CFendl;

  Uint nb_ghosts=0;

  CMeshWriter::Ptr gmsh_writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->write_from_to(mesh,"line.msh");

  BOOST_CHECK(true);

  CFinfo << mesh.tree() << CFendl;

  Geometry& nodes = mesh.geometry();
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

  CMeshGenerator::Ptr meshgenerator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","1Dgenerator");

  meshgenerator->configure_option("parent",URI("//Root"));
  meshgenerator->configure_option("name",std::string("rect"));
  meshgenerator->configure_option("nb_cells",std::vector<Uint>(2,2));
  meshgenerator->configure_option("lengths",std::vector<Real>(2,2.));
  meshgenerator->execute();
  CMesh& mesh = Core::instance().root().get_child("rect").as_type<CMesh>();



  CFinfo << "elements count = " << mesh.topology().recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << mesh.topology().recursive_nodes_count() << CFendl;

  Uint nb_ghosts=0;

  CMeshWriter::Ptr gmsh_writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->write_from_to(mesh,"rect.msh");

  BOOST_CHECK(true);

  CFinfo << mesh.tree() << CFendl;

  Geometry& nodes = mesh.geometry();
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
  MPI::PE::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

