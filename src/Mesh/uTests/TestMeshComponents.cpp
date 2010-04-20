#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct MeshComponent_Fixture
{
  /// common setup for each test case
  MeshComponent_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~MeshComponent_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshComponent_TestSuite, MeshComponent_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( MeshComponentTest )
{
  CFinfo << "testing MeshComponents " << CFendl;

  // Create mesh component
  CMesh mesh ( "mesh" );
  BOOST_CHECK_EQUAL ( mesh.name() , "mesh" );
  BOOST_CHECK_EQUAL ( mesh.path().string() , "" );
  BOOST_CHECK_EQUAL ( mesh.full_path().string() , "mesh" );
  
  // Create one region inside mesh
  mesh.create_region("region1");
  SafePtr<CRegion> region1 = mesh.get_component("region1").d_castTo<CRegion>();
  BOOST_CHECK_EQUAL ( region1->full_path().string() , "mesh/region1" );

  // Create second region inside mesh, with 2 subregions inside
  mesh.create_region("region2");
  SafePtr<CRegion> region2 = mesh.get_component("region2").d_castTo<CRegion>();
  region2->create_region("subregion1");
  region2->create_region("subregion2");
  BOOST_CHECK_EQUAL ( region2->get_component("subregion2")->full_path().string() , "mesh/region2/subregion2" );

  // Create a connectivity table inside a subregion
  SafePtr<CRegion> subregion = region2->get_component("subregion2").d_castTo<CRegion>();
  subregion->create_connectivityTable("connTable");
  BOOST_CHECK_EQUAL ( subregion->get_component("connTable")->full_path().string() , "mesh/region2/subregion2/connTable" );
  
  // Create an array of coordinates inside mesh
  mesh.create_array("coordinates");
  BOOST_CHECK_EQUAL ( mesh.get_component("coordinates")->full_path().string() , "mesh/coordinates" );

  
  
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

