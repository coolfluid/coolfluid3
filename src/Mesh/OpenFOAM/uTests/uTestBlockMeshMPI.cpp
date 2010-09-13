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

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::OpenFOAM;

//////////////////////////////////////////////////////////////////////////////

struct BlockMeshMPI_Fixture
{
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( BlockMeshMPI, BlockMeshMPI_Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( PartitionBlocks )
{
  // Use SYNC_SCREEN logging only
  Logger::instance().getStream(Logger::INFO).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::ERROR).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::WARN).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::DEBUG).useDestination(LogStream::SCREEN, false);
  Logger::instance().getStream(Logger::TRACE).useDestination(LogStream::SCREEN, false);
   
  CF::AssertionManager::instance().AssertionThrows = true;
  try
  {
    PEInterface::instance().init(0,0);
    
    const CF::Uint rank = PEInterface::instance().rank();
    
    if(rank == 10000 || rank == 2000000)
    {
      int i = 0;
      std::cout << "PID " << getpid() << " ready for attach" << std::endl;
      while (0 == i)
        sleep(5);
    }
    
    boost::filesystem::path path = boost::filesystem::path("channel3d.dict");
    boost::filesystem::fstream file;
    file.open(path,std::ios_base::in);
    
    BlockData block_data;
    parse_blockmesh_dict(file, block_data);
    
    BlockData partitioned_blocks;
    partition_blocks(block_data, PEInterface::instance().size(), CF::XX, partitioned_blocks);
    
    // Generate the parallel mesh
    CMesh::Ptr partitioned_mesh(new CMesh("partitioned_mesh"));
    SimpleCommunicationPattern pattern;
    SimpleCommunicationPattern::IndicesT nodes_dist;
    build_mesh(partitioned_blocks, *partitioned_mesh, nodes_dist);
    
    make_receive_lists(nodes_dist, *partitioned_mesh, pattern);
    pattern.update_send_lists();
    apply_pattern_carray(pattern, recursive_range_typed<CArray>(*partitioned_mesh));
    
    CF::Real volume = 0;
    
    BOOST_FOREACH(const CElements& celements, recursive_filtered_range_typed<CElements>(*partitioned_mesh, IsElementsVolume()))
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
  catch(std::exception& E)
  {
    std::cout << "Exception in process " << PEInterface::instance().rank() << ": " << E.what() << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

