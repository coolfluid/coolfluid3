// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::neu::Reader"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshTransformer.hpp"

#include "common/DynTable.hpp"
#include "common/List.hpp"
#include "common/Table.hpp"
#include "mesh/Dictionary.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct neuReaderMPITests_Fixture
{
  /// common setup for each test case
  neuReaderMPITests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~neuReaderMPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( neuReaderMPITests_TestSuite, neuReaderMPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  PE::Comm::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  meshreader->options().set("read_groups",true);

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("quadtriag");

  meshreader->read_mesh_into("../../resources/quadtriag.neu",mesh);


  CFinfo << "elements count = " << find_component<Region>(mesh).recursive_elements_count(true) << CFendl;
  CFinfo << "nodes count    = " << find_component<Region>(mesh).recursive_nodes_count() << CFendl;

  Uint nb_ghosts=0;

  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,"quadtriag.msh");

  BOOST_CHECK(true);

  CFinfo << mesh.tree() << CFendl;

  Dictionary& nodes = find_component_recursively<Dictionary>(mesh);
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
/*
BOOST_AUTO_TEST_CASE( threeD_test )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  meshreader->options().set("number_of_processors",(Uint) Comm::instance().size());
  meshreader->options().set("rank",(Uint) Comm::instance().rank());
  meshreader->options().set("Repartition",false);
  meshreader->options().set("OutputRank",(Uint) 2);

  // the file to read from
  boost::filesystem::path fp_in ("../../resources/hextet.neu");

  // the mesh to store in
  boost::shared_ptr< Mesh > mesh ( allocate_component<Mesh>  ( "mesh" ) );


  CFinfo.setFilterRankZero(false);
  meshreader->do_read_mesh_into(fp_in,mesh);
  CFinfo.setFilterRankZero(true);

  boost::filesystem::path fp_out ("hextet.msh");
  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);

  BOOST_CHECK(true);

}
*/
////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( read_multiple_2D )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  meshreader->options().set("Repartition",true);
  meshreader->options().set("OutputRank",(Uint) 0);

  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");

  // the mesh to store in
  boost::shared_ptr< Mesh > mesh ( allocate_component<Mesh>  ( "mesh" ) );


  CFinfo.setFilterRankZero(false);



  for (Uint count=1; count<=2; ++count)
  {
    CFinfo << "\n\n\nMesh parallel:" << CFendl;
    meshreader->do_read_mesh_into(fp_in,mesh);
  }



  CFinfo.setFilterRankZero(true);
  CFinfo << mesh->tree() << CFendl;
  CFinfo << meshreader->tree() << CFendl;
  boost::shared_ptr< MeshTransformer > info  = build_component_abstract_type<MeshTransformer>("Info","info");
  info->transform(mesh);


  boost::filesystem::path fp_out ("quadtriag_mult.msh");
  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);

  BOOST_CHECK_EQUAL(1,1);

}
*/
////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

