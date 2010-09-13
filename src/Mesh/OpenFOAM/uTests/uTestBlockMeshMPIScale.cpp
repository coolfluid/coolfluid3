#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::OpenFOAM::BlockMeshMPI"

#include <boost/assign/list_of.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/ElementNodes.hpp"

#include "Mesh/OpenFOAM/BlockData.hpp"
#include "Mesh/OpenFOAM/Parser.hpp"
#include "Mesh/OpenFOAM/SimpleCommunicationPattern.hpp"
#include "Mesh/OpenFOAM/WriteDict.hpp"

#include "Mesh/SF/Hexa3DLagrangeP1.hpp"

#include "Tools/MeshDiff/MeshDiff.hpp"

#include "Tools/Testing/ProfiledTestFixture.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::OpenFOAM;
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
  PEInterface::instance().init(0,0);
  pattern = SimpleCommunicationPattern(); // must be created after MPI init
  // Use SYNC_SCREEN logging only
  Logger::instance().getStream(Logger::INFO).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::ERROR).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::WARN).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::DEBUG).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::TRACE).useDestination(LogStream::SCREEN, false);
  
  CFinfo << "Created global fixture" << CFendl;
  
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
    nodes_dist(GlobalFixture::instance->nodes_dist)
  {
  }
  
  BlockData& block_data;
  BlockData& partitioned_blocks;
  CMesh& partitioned_mesh;
  SimpleCommunicationPattern& pattern;
  SimpleCommunicationPattern::IndicesT& nodes_dist;
};


BOOST_GLOBAL_FIXTURE( GlobalFixture )

BOOST_FIXTURE_TEST_SUITE( BlockMeshMPI, BlockMeshMPIFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( PartitionBlocksScale )
{
  boost::filesystem::path path = boost::filesystem::path("channel3d.dict");
  boost::filesystem::fstream file;
  file.open(path,std::ios_base::in);
  
  parse_blockmesh_dict(file, block_data);
  
  // Multiply the data, so each CPU has enough work, and keep size per CPU constant
  const Uint nb_procs = PEInterface::instance().size();
  const Uint factor = 10;
  BOOST_FOREACH(BlockData::CountsT& subdivisions, block_data.block_subdivisions)
  {
    for(Uint i = 0; i != subdivisions.size(); ++i)
      subdivisions[i] *= (i == 0 ? factor*nb_procs : factor);
  }
  
  partition_blocks(block_data, PEInterface::instance().size(), CF::XX, partitioned_blocks);
}

BOOST_AUTO_TEST_CASE( BuildMeshScale )
{
  // Generate the parallel mesh
  build_mesh(partitioned_blocks, partitioned_mesh, nodes_dist);
}

BOOST_AUTO_TEST_CASE( BuildReceiveListsScale )
{
  make_node_receive_lists(nodes_dist, partitioned_mesh, pattern);
}

BOOST_AUTO_TEST_CASE( UpdateSendListsScale )
{
  pattern.update_send_lists();
}

BOOST_AUTO_TEST_CASE( ApplyCommPatternScale )
{
  apply_pattern_carray(pattern, recursive_range_typed<CArray>(partitioned_mesh));
}

BOOST_AUTO_TEST_CASE( ComputeVolumeScale )
{
  CFinfo << "Computing volume" << CFendl;
  CF::Real volume = 0;
    
  BOOST_FOREACH(const CElements& celements, recursive_filtered_range_typed<CElements>(partitioned_mesh, IsElementsVolume()))
  {
    const CArray& coords = celements.coordinates();
    const CTable::ArrayT& conn_table = celements.connectivity_table().array();
    BOOST_FOREACH(const CTable::ConstRow row, conn_table)
    {
      const CF::Real elem_vol = SF::Hexa3DLagrangeP1::volume(ConstElementNodeView(coords, row));
      volume += elem_vol;
    }
  }
  
  CFinfo << "Volume for process " << PEInterface::instance().rank() << ": " << volume << CFendl;
  
  if (PEInterface::instance().rank() == 0)
  {
    Real total_volume;
    boost::mpi::reduce(PEInterface::instance(), volume, total_volume, std::plus<Real>(), 0);
    BOOST_CHECK_CLOSE(total_volume, 78.9568, 0.0001);
  }
  else
  {
    boost::mpi::reduce(PEInterface::instance(), volume, std::plus<Real>(), 0);
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

