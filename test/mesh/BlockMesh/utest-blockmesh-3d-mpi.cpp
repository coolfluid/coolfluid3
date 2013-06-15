// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::BlockMesh::BlockMeshMPI"

#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Core.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"

#include "common/PE/Comm.hpp"

#include "mesh/BlockMesh/BlockData.hpp"
#include "mesh/Domain.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Field.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Dictionary.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;

using namespace boost::assign;

//////////////////////////////////////////////////////////////////////////////

struct BockMesh3DFixture :
  public Tools::Testing::TimedTestFixture
{
  BockMesh3DFixture()
  {
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char** argv = boost::unit_test::framework::master_test_suite().argv;

    cf3_assert(argc >= 4);
    x_segs = boost::lexical_cast<Uint>(argv[1]);
    y_segs = boost::lexical_cast<Uint>(argv[2]);
    z_segs = boost::lexical_cast<Uint>(argv[3]);

    if(!PE::Comm::instance().is_active())
      PE::Comm::instance().init(argc, argv);

    Component& root = Core::instance().root();
    if(!root.get_child("domain"))
    {
      m_domain = root.create_component<Domain>("domain");
    }
    else
    {
      m_domain = Handle<Domain>(root.get_child("domain"));
    }

    if(!domain().get_child("mesh"))
    {
      m_mesh = domain().create_component<Mesh>("mesh");
    }
    else
    {
      m_mesh = Handle<Mesh>(domain().get_child("mesh"));
    }
  }

  Domain& domain()
  {
    return *m_domain;
  }

  Mesh& mesh()
  {
    return *m_mesh;
  }

  Uint x_segs, y_segs, z_segs;

  Handle<Domain> m_domain;
  Handle<Mesh> m_mesh;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( BlockMesh3D, BockMesh3DFixture )

//////////////////////////////////////////////////////////////////////////////

std::vector<URI> fields;

BOOST_AUTO_TEST_CASE( Setup )
{
  // Make sure MPI is up before running the first test
  BOOST_CHECK(PE::Comm::instance().is_active());
}

BOOST_AUTO_TEST_CASE( GenerateMesh )
{
  const Uint nb_procs = PE::Comm::instance().size();
  const Uint rank = PE::Comm::instance().rank();

  const Real length = 12.;
  const Real half_height = 0.5;
  const Real width = 6.;
  const Real ratio = 0.1;

  BlockMesh::BlockArrays& blocks = *domain().create_component<BlockMesh::BlockArrays>("BlockArrays");

  // Create blocks for a 3D channel
  Tools::MeshGeneration::create_channel_3d(blocks, length, half_height, width, x_segs, y_segs/2, z_segs, ratio);

  // Try partitioning in multiple directions for certain numbers of CPUS
  if(nb_procs == 16)
  {
    blocks.partition_blocks(8, XX);
    blocks.partition_blocks(2, ZZ);
  }
  else if(nb_procs == 32)
  {
    blocks.partition_blocks(8, XX);
    blocks.partition_blocks(4, ZZ);
  }
  else if(nb_procs == 64)
  {
    blocks.partition_blocks(16, XX);
    blocks.partition_blocks(4, ZZ);
  }
  else if(nb_procs == 128)
  {
    blocks.partition_blocks(16, XX);
    blocks.partition_blocks(8, ZZ);
  }
  else
  {
    blocks.partition_blocks(nb_procs, XX);
  }
  
  blocks.create_mesh(mesh());
}

BOOST_AUTO_TEST_CASE( RankField )
{
  // Store element ranks
  Dictionary& elems_P0 = mesh().create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0");
  Field& elem_rank = elems_P0.create_field("elem_rank");

  boost_foreach(const Handle<Entities>& elements_handle, elems_P0.entities_range())
  {
    Entities& elements = *elements_handle;
    const Space& space = elems_P0.space(elements);
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      Uint field_idx = space.connectivity()[elem][0];
      elem_rank[field_idx][0] = elements.rank()[elem];
    }
  }
  fields.push_back(elem_rank.uri());
}

BOOST_AUTO_TEST_CASE( WriteMesh )
{
  mesh().write_mesh("utest-blockmesh-3d-mpi_output.pvtu", fields);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

