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
#include "common/CRoot.hpp"
#include "common/PE/Comm.hpp"

#include "mesh/Blockmesh/BlockData.hpp"
#include "mesh/CDomain.hpp"
#include "mesh/CElements.hpp"
#include "mesh/CMesh.hpp"
#include "mesh/CMeshWriter.hpp"
#include "mesh/CRegion.hpp"
#include "mesh/CSpace.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldGroup.hpp"

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

    CRoot& root = Core::instance().root();
    if(!root.get_child_ptr("domain"))
    {
      m_domain = root.create_component_ptr<CDomain>("domain");
    }
    else
    {
      m_domain = root.get_child("domain").as_ptr<CDomain>();
    }

    if(!domain().get_child_ptr("writer"))
    {
      m_writer = domain().add_component(build_component_abstract_type<CMeshWriter>("CF.Mesh.VTKXML.CWriter", "writer")).as_ptr<CMeshWriter>();
    }
    else
    {
      m_writer = domain().get_child("writer").as_ptr<CMeshWriter>();
    }

    if(!domain().get_child_ptr("block_mesh"))
    {
      m_block_mesh = domain().create_component_ptr<CMesh>("block_mesh");
    }
    else
    {
      m_block_mesh = domain().get_child("block_mesh").as_ptr<CMesh>();
    }

    if(!domain().get_child_ptr("mesh"))
    {
      m_mesh = domain().create_component_ptr<CMesh>("mesh");
    }
    else
    {
      m_mesh = domain().get_child("mesh").as_ptr<CMesh>();
    }

    if(argc == 5)
    {
      writer().configure_option("distributed_files", true);
      base_dir = std::string(argv[4]) + "/";
    }

  }

  CDomain& domain()
  {
    return *m_domain.lock();
  }

  CMeshWriter& writer()
  {
    return *m_writer.lock();
  }

  CMesh& block_mesh()
  {
    return *m_block_mesh.lock();
  }

  CMesh& mesh()
  {
    return *m_mesh.lock();
  }

  Uint x_segs, y_segs, z_segs;

  boost::weak_ptr<CDomain> m_domain;
  boost::weak_ptr<CMesh> m_block_mesh; // Mesh containing the blocks (helper for parallelization)
  boost::weak_ptr<CMesh> m_mesh; // Actual generated mesh
  boost::weak_ptr<CMeshWriter> m_writer;

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

  BlockMesh::BlockData& serial_blocks = domain().create_component<BlockMesh::BlockData>("serial_blocks");

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
    BlockMesh::BlockData& parallel_blocks_x = domain().create_component<BlockMesh::BlockData>("parallel_blocks_x");
    BlockMesh::BlockData& parallel_blocks_z = domain().create_component<BlockMesh::BlockData>("parallel_blocks_z");

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
    BlockMesh::BlockData& parallel_blocks = domain().create_component<BlockMesh::BlockData>("parallel_blocks");
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
  FieldGroup& elems_P0 = mesh().create_space_and_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED,"CF.Mesh.LagrangeP0");
  Field& elem_rank = elems_P0.create_field("elem_rank");

  boost_foreach(CElements& elements , elems_P0.elements_range())
  {
    CSpace& space = elems_P0.space(elements);
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      Uint field_idx = space.indexes_for_element(elem)[0];
      elem_rank[field_idx][0] = elements.rank()[elem];
    }
  }

  // setup fields to write
  std::vector<Field::Ptr> fields;
  fields.push_back(mesh().geometry().coordinates().as_ptr<Field>());
  fields.push_back(elem_rank.as_ptr<Field>());
  writer().set_fields(fields);
}

BOOST_AUTO_TEST_CASE( WriteMesh )
{
  writer().write_from_to(mesh(), base_dir + "utest-blockmesh-3d-mpi_output.pvtu");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

