#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/ElementType.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct MeshConstruction_Fixture
{
  /// common setup for each test case
  MeshConstruction_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~MeshConstruction_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  std::vector<Real> create_coord(const Real& x, const Real& y) {
    Real coord[] = {x,y};
    std::vector<Real> coordVec;
    coordVec.assign(coord,coord+2);
    return coordVec;
  }
  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshConstruction_TestSuite, MeshConstruction_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( MeshConstruction )
{

  // Create root and mesh component
  boost::shared_ptr<Component> root ( new CRoot  ( "root" ) );
  boost::shared_ptr<Component> mesh ( new CMesh  ( "mesh" ) );

  root->add_component( mesh );
  
  // create a mesh pointer
  boost::shared_ptr<CMesh> p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  // create regions
  p_mesh->create_region("superRegion");
  p_mesh->get_component("superRegion").d_castTo<CRegion>()->create_region("quads");
  p_mesh->get_component("superRegion").d_castTo<CRegion>()->create_region("triags");
  
  // create a pointer to the quads region
  SafePtr<CRegion> quadRegion = 
    p_mesh->get_component("superRegion")->get_component("quads").d_castTo<CRegion>(); 
  
  // create connectivity table and element type in the quads region
  quadRegion->create_connectivityTable("table");
  quadRegion->create_elementType("type");
  
  // create a coordinates array in the mesh component
  p_mesh->create_array("coordinates");
  
  
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

