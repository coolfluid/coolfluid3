// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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
#include "mesh/SpaceFields.hpp"

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

    if(!domain().get_child("writer"))
    {
      m_writer = domain().add_component(build_component_abstract_type<MeshWriter>("cf3.mesh.VTKXML.Writer", "writer")).handle<MeshWriter>();
    }
    else
    {
      m_writer = Handle<MeshWriter>(domain().get_child("writer"));
    }

    if(!domain().get_child("block_mesh"))
    {
      m_block_mesh = domain().create_component<Mesh>("block_mesh");
    }
    else
    {
      m_block_mesh = Handle<Mesh>(domain().get_child("block_mesh"));
    }

    if(!domain().get_child("mesh"))
    {
      m_mesh = domain().create_component<Mesh>("mesh");
    }
    else
    {
      m_mesh = Handle<Mesh>(domain().get_child("mesh"));
    }

    if(argc == 5)
    {
      writer().options().configure_option("distributed_files", true);
      base_dir = std::string(argv[4]) + "/";
    }

  }

  Domain& domain()
  {
    return *m_domain;
  }

  MeshWriter& writer()
  {
    return *m_writer;
  }

  Mesh& block_mesh()
  {
    return *m_block_mesh;
  }

  Mesh& mesh()
  {
    return *m_mesh;
  }

  Uint x_segs, y_segs, z_segs;

  Handle<Domain> m_domain;
  Handle<Mesh> m_block_mesh; // Mesh containing the blocks (helper for parallelization)
  Handle<Mesh> m_mesh; // Actual generated mesh
  Handle<MeshWriter> m_writer;

  std::string base_dir;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( BlockMesh3D, BockMesh3DFixture )

//////////////////////////////////////////////////////////////////////////////

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

  BlockMesh::BlockData& serial_blocks = *domain().create_component<BlockMesh::BlockData>("serial_blocks");

  // Create blocks for a 3D channel
  Tools::MeshGeneration::create_channel_3d(serial_blocks, length, half_height, width, x_segs, y_segs/2, z_segs, ratio);

  // Try partitioning in multiple directions for certain numbers of CPUS
  Uint nb_x_partitions = 0;
  Uint nb_z_partitions = 0;

  if(nb_procs == 16)
  {
    nb_x_partitions = 4;
    nb_z_partitions = 2;
  }
  else if(nb_procs == 32)
  {
    nb_x_partitions = 8;
    nb_z_partitions = 2;
  }
  else if(nb_procs == 64)
  {
    nb_x_partitions = 8;
    nb_z_partitions = 4;
  }
  else if(nb_procs == 128)
  {
    nb_x_partitions = 16;
    nb_z_partitions = 4;
  }

  if(nb_z_partitions != 0)
  {
    BlockMesh::BlockData& parallel_blocks_x = *domain().create_component<BlockMesh::BlockData>("parallel_blocks_x");
    BlockMesh::BlockData& parallel_blocks_z = *domain().create_component<BlockMesh::BlockData>("parallel_blocks_z");

    BlockMesh::partition_blocks(serial_blocks, nb_x_partitions, XX, parallel_blocks_x);
    BlockMesh::partition_blocks(parallel_blocks_x, nb_z_partitions, ZZ, parallel_blocks_z);

    BOOST_CHECK_EQUAL(parallel_blocks_z.block_points.size(), nb_procs);
    parallel_blocks_z.block_distribution.resize(nb_procs+1);
    for(Uint i = 0; i != nb_procs; ++i)
      parallel_blocks_z.block_distribution[i] = i;
    parallel_blocks_z.block_distribution[nb_procs] = nb_procs;

    // Gnerate the actual mesh
    BlockMesh::build_mesh(parallel_blocks_z, mesh());
  }
  else
  {
    BlockMesh::BlockData& parallel_blocks = *domain().create_component<BlockMesh::BlockData>("parallel_blocks");
    // partition blocks
    BlockMesh::partition_blocks(serial_blocks, nb_procs, XX, parallel_blocks);

    // Generate the actual mesh
    BlockMesh::build_mesh(parallel_blocks, mesh());
    std::cout << "done for " << rank << std::endl;
  }
}

BOOST_AUTO_TEST_CASE( RankField )
{
  // Store element ranks
  SpaceFields& elems_P0 = mesh().create_space_and_field_group("elems_P0",SpaceFields::Basis::ELEMENT_BASED,"cf3.mesh.LagrangeP0");
  Field& elem_rank = elems_P0.create_field("elem_rank");

  boost_foreach(const Handle<Entities>& elements_handle, elems_P0.entities_range())
  {
    Entities& elements = *elements_handle;
    Space& space = elems_P0.space(elements);
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      Uint field_idx = space.indexes_for_element(elem)[0];
      elem_rank[field_idx][0] = elements.rank()[elem];
    }
  }

  // setup fields to write
  std::vector<Handle< Field > > fields;
  fields.push_back(mesh().geometry_fields().coordinates().handle<Field>());
  fields.push_back(elem_rank.handle<Field>());
  writer().set_fields(fields);
}

BOOST_AUTO_TEST_CASE( WriteMesh )
{
  writer().write_from_to(mesh(), base_dir + "utest-blockmesh-3d-mpi_output.pvtu");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

