// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::BlockMesh::BlockMeshMPI"

#include <boost/assign/list_of.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/mpi/collectives.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CTable.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CNodes.hpp"

#include "Mesh/BlockMesh/BlockData.hpp"
#include "Mesh/SimpleCommunicationPattern.hpp"
#include "Mesh/BlockMesh/WriteDict.hpp"

#include "Mesh/SF/Hexa3DLagrangeP1.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Common::mpi;
using namespace CF::Mesh;
using namespace CF::Mesh::BlockMesh;
using namespace CF::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

struct GlobalFixture
{
  GlobalFixture();
  
  // Global data items
  BlockData block_data;
  BlockData partitioned_blocks;
  CMesh::Ptr partitioned_mesh;
  SimpleCommunicationPattern pattern;
  SimpleCommunicationPattern::IndicesT nodes_dist;
  
  static GlobalFixture* instance;
};

GlobalFixture* GlobalFixture::instance = 0;

GlobalFixture::GlobalFixture() : partitioned_mesh(new CMesh("partitioned_mesh"))
{
  PE::instance().init(0,0);
  
  pattern = SimpleCommunicationPattern(); // must be created after MPI init
  // Use SYNC_SCREEN logging only
  Logger::instance().getStream(Logger::INFO).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::ERROR).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::WARN).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::DEBUG).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::TRACE).useDestination(LogStream::SCREEN, false);
	Logger::instance().getStream(Logger::INFO).useDestination(LogStream::SYNC_SCREEN, false);
  Logger::instance().getStream(Logger::ERROR).useDestination(LogStream::SYNC_SCREEN, false);
  Logger::instance().getStream(Logger::WARN).useDestination(LogStream::SYNC_SCREEN, false);
  Logger::instance().getStream(Logger::DEBUG).useDestination(LogStream::SYNC_SCREEN, false);
  Logger::instance().getStream(Logger::TRACE).useDestination(LogStream::SYNC_SCREEN, false);
	Logger::instance().getStream(Logger::INFO).setFilterRankZero(LogStream::SYNC_SCREEN, false);
  Logger::instance().getStream(Logger::ERROR).setFilterRankZero(LogStream::SYNC_SCREEN, false);
  Logger::instance().getStream(Logger::WARN).setFilterRankZero(LogStream::SYNC_SCREEN, false);
  Logger::instance().getStream(Logger::DEBUG).setFilterRankZero(LogStream::SYNC_SCREEN, false);
  Logger::instance().getStream(Logger::TRACE).setFilterRankZero(LogStream::SYNC_SCREEN, false);
  instance = this;
}

//////////////////////////////////////////////////////////////////////////////

struct BlockMeshMPIFixture :
  public TimedTestFixture
{
  BlockMeshMPIFixture() :
    block_data(GlobalFixture::instance->block_data),
    partitioned_blocks(GlobalFixture::instance->partitioned_blocks),
    partitioned_mesh(*GlobalFixture::instance->partitioned_mesh),
    pattern(GlobalFixture::instance->pattern),
    nodes_dist(GlobalFixture::instance->nodes_dist),
    factor(2),
    scale(false)
  {
    int    argc = boost::unit_test::framework::master_test_suite().argc;
    char** argv = boost::unit_test::framework::master_test_suite().argv;
    
    if(argc > 1)
    {
      factor = boost::lexical_cast<Uint>(argv[1]);
      scale =  argc > 2 ? std::string(argv[2]) == "scale" : false;
    }
    
    CFinfo << "Using factor " << factor << ", " << (scale ? "" : "not ") << "scaled" << CFendl;
  }
  
  BlockData& block_data;
  BlockData& partitioned_blocks;
  CMesh& partitioned_mesh;
  SimpleCommunicationPattern& pattern;
  SimpleCommunicationPattern::IndicesT& nodes_dist;
  
  Uint factor;
  bool scale;
};


BOOST_GLOBAL_FIXTURE( GlobalFixture )

BOOST_FIXTURE_TEST_SUITE( BlockMeshMPI, BlockMeshMPIFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( PartitionBlocks )
{
  Tools::MeshGeneration::create_channel_3d(block_data, 12.5663706144, 0.5, 6.28318530718, 16, 8, 12, 0.1);
  
  // Multiply the data, so each CPU has enough work, and keep size per CPU constant
  const Uint nb_procs = PE::instance().size();
  
  BOOST_FOREACH(BlockData::CountsT& subdivisions, block_data.block_subdivisions)
  {
    for(Uint i = 0; i != subdivisions.size(); ++i)
      subdivisions[i] *= ((i == 0 && scale) ? factor*nb_procs : factor);
  }
  
  partition_blocks(block_data, PE::instance().size(), XX, partitioned_blocks);
}

BOOST_AUTO_TEST_CASE( BuildMesh )
{
  // Generate the parallel mesh
  build_mesh(partitioned_blocks, partitioned_mesh, nodes_dist);
}

BOOST_AUTO_TEST_CASE( BuildReceiveLists )
{
  make_node_receive_lists(nodes_dist, partitioned_mesh, pattern);
}

BOOST_AUTO_TEST_CASE( UpdateSendLists )
{
  pattern.update_send_lists();
}

BOOST_AUTO_TEST_CASE( ApplyCommPattern )
{
  apply_pattern_CTable(pattern, find_components_recursively<CTable<Real> >(partitioned_mesh));
}

BOOST_AUTO_TEST_CASE( ComputeVolume )
{
  CF::Real volume = 0;
  Uint nb_elems = 0;
  
  const Uint rank = PE::instance().rank();
  const Uint nb_procs = PE::instance().size();
  
  // This assumes a channel mesh with 2 transversal blocks as input
  const Uint slice_size = 2*block_data.block_subdivisions[0][YY] * block_data.block_subdivisions[0][ZZ];
  const Uint mesh_length = block_data.block_subdivisions[0][XX];
  const Uint elem_length = ( (mesh_length % nb_procs) == 0 || rank != (nb_procs-1) ) ? mesh_length / nb_procs : mesh_length % nb_procs;
  
  SF::Hexa3DLagrangeP1::NodeMatrixT nodes;
  BOOST_FOREACH(const CElements& celements, find_components_recursively_with_filter<CElements>(partitioned_mesh, IsElementsVolume()))
  {
    const CTable<Real>& coords = celements.nodes().coordinates();
    const CTable<Uint>::ArrayT& conn_table = celements.connectivity_table().array();
    BOOST_FOREACH(const CTable<Uint>::ConstRow row, conn_table)
    {
      fill(nodes, coords, row);
      const CF::Real elem_vol = SF::Hexa3DLagrangeP1::volume(nodes);
      volume += elem_vol;
      ++nb_elems;
    }
  }
  
  BOOST_CHECK_EQUAL(slice_size*elem_length, nb_elems);
  BOOST_CHECK_CLOSE(volume, 78.9568 * static_cast<Real>(elem_length) / static_cast<Real>(mesh_length), 0.5);
  
  CFinfo << "Number of elements for process " << rank << ": " << nb_elems << CFendl;
  CFinfo << "Volume for process " << rank << ": " << volume << CFendl;
  
  if (PE::instance().rank() == 0)
  {
    Real total_volume;
    ///@todo remove boost::mpi calls
    boost::mpi::communicator world;
    boost::mpi::reduce(world, volume, total_volume, std::plus<Real>(), 0);
    BOOST_CHECK_CLOSE(total_volume, 78.9568, 0.5);
  }
  else
  {
    Real total_volume;
    ///@todo remove boost::mpi calls
    boost::mpi::communicator world;
    boost::mpi::reduce(world, volume, std::plus<Real>(), 0);
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

