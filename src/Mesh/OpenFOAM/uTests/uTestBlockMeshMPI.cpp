#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::OpenFOAM::BlockMeshMPI"

#include <boost/assign/list_of.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Mesh/OpenFOAM/BlockData.hpp"
#include "Mesh/OpenFOAM/Parser.hpp"
#include "Mesh/OpenFOAM/SimpleCommunicationPattern.hpp"
#include "Mesh/OpenFOAM/WriteDict.hpp"

#include "Tools/MeshDiff/MeshDiff.hpp"

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
  CF::AssertionManager::instance().AssertionThrows = true;
  try
  {
    PEInterface::instance().init(0,0);
    const CF::Uint rank = PEInterface::instance().rank();
    
    if(rank == 1000000 || rank == 2000000)
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
    
    //if( PEInterface::instance().rank() == 0)
    //  std::cout << "-------------- Partitioned blocks ----------------\n" << partitioned_blocks << std::endl;
    
    // Generate the parallel mesh
    CMesh::Ptr partitioned_mesh(new CMesh("partitioned_mesh"));
    SimpleCommunicationPattern pattern;
    build_mesh(partitioned_blocks, *partitioned_mesh, pattern);
    std::cout << "-------------- Pattern for process " << PEInterface::instance().rank() << " ----------------\n" << pattern << std::endl;
  }
  catch(std::exception& E)
  {
    std::cout << "Exception in process " << PEInterface::instance().rank() << ": " << E.what() << std::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

