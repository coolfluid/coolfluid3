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
  
  std::vector<Uint> create_quad(const Uint& A, const Uint& B, const Uint& C, const Uint& D) {
    Uint quad[] = {A,B,C,D};
    std::vector<Uint> quadVec;
    quadVec.assign(quad,quad+4);
    return quadVec;
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
  
  // create pointers to the coordinates array and connectivity table
  SafePtr<CArray::Array> coordinates = (&p_mesh->get_component("coordinates").d_castTo<CArray>()->getArray());
  SafePtr<CTable::ConnectivityTable> qTable = (&quadRegion->get_component("table").d_castTo<CTable>()->getTable());
  
  // initialize the coordinates array and connectivity table
  coordinates->initialize(2,20);
  qTable->initialize(4,10);
  
  // fill coordinates in the buffer
  coordinates->add_row(create_coord( 0.0 , 0.0 ));  // 0
  coordinates->add_row(create_coord( 1.0 , 0.0 ));  // 1
  coordinates->add_row(create_coord( 1.0 , 1.0 ));  // 2
  coordinates->add_row(create_coord( 0.0 , 1.0 ));  // 3
  coordinates->add_row(create_coord( 0.0 , 2.0 ));  // 4
  coordinates->add_row(create_coord( 1.0 , 1.0 ));  // 5
  
  // fill connectivity in the buffer
  qTable->add_row(create_quad( 0 , 1 , 2 , 3));
  qTable->add_row(create_quad( 3 , 2 , 4 , 5));

  // flush buffers into the table. 
  // This causes the table and array to be resized and filled.
  coordinates->flush();
  qTable->flush();
  
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

