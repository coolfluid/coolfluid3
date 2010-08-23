#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::OpenFOAM::BlockMeshReader"

#include <boost/assign/list_of.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Mesh/OpenFOAM/BlockMeshReader.hpp"

using namespace CF::Mesh::OpenFOAM;

//////////////////////////////////////////////////////////////////////////////

struct BlockMeshReader_Fixture
{
  /// common setup for each test case
  BlockMeshReader_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

    //mapped_coords += 0.1, 0.8;
  }

  /// common tear-down for each test case
  ~BlockMeshReader_Fixture()
  {
  }

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( BlockMeshReader, BlockMeshReader_Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ReadFile )
{
  BlockData blockData;

  boost::filesystem::fstream file;

  // file should be in current working directory
  boost::filesystem::path fp = boost::filesystem::path("blockMesh.dict");
  if( boost::filesystem::exists(fp) )
  {
     file.open(fp,std::ios_base::in);
  }
  else // doesnt exist so throw exception
  {
     throw boost::filesystem::filesystem_error( fp.string() + " does not exist",
                                                boost::system::error_code() );
  }

  readBlockMeshFile(file, blockData);

  BOOST_CHECK_EQUAL(blockData.scaling_factor, 1);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

