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

  std::vector<Real> create_coord(const Real& x, const Real& y) {
    Real coord[] = {x,y};
    std::vector<Real> coordVec;
    coordVec.assign(coord,coord+2);
    return coordVec;
  }
  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshComponent_TestSuite, MeshComponent_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( MeshComponentTest )
{
  // CFinfo << "testing MeshComponents \n" << CFendl;

  // Create root and mesh component
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> mesh ( new CMesh  ( "mesh" ) );

  root->add_component( mesh );

  BOOST_CHECK_EQUAL ( mesh->name() , "mesh" );
  BOOST_CHECK_EQUAL ( mesh->path().string() , "//root" );
  BOOST_CHECK_EQUAL ( mesh->full_path().string() , "//root/mesh" );
  
  // Create one region inside mesh
  boost::shared_ptr<CMesh> p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  p_mesh->create_region("region1");
  SafePtr<CRegion> region1 = p_mesh->get_component<CRegion>("region1");
  BOOST_CHECK_EQUAL ( region1->full_path().string() , "//root/mesh/region1" );

  // Create second region inside mesh, with 2 subregions inside
  p_mesh->create_region("region2");
  SafePtr<CRegion> region2 = p_mesh->get_component<CRegion>("region2");
  region2->create_region("subregion1");
  region2->create_region("subregion2");
  BOOST_CHECK_EQUAL ( region2->get_component("subregion2")->full_path().string() , "//root/mesh/region2/subregion2" );

  // Create a connectivity table inside a subregion
  SafePtr<CRegion> subregion = region2->get_component<CRegion>("subregion2");
  subregion->create_connectivityTable("connTable");
  BOOST_CHECK_EQUAL ( subregion->get_component("connTable")->full_path().string() , "//root/mesh/region2/subregion2/connTable" );
  
  // Create a elementsType component inside a subregion
  subregion->create_elementType("elementType");
  BOOST_CHECK_EQUAL ( subregion->get_component("elementType")->full_path().string() , "//root/mesh/region2/subregion2/elementType" );
  
  
  // Create an array of coordinates inside mesh
  p_mesh->create_array("coordinates");
  BOOST_CHECK_EQUAL ( p_mesh->get_component("coordinates")->full_path().string() , "//root/mesh/coordinates" );
  
  region2->get_component<CRegion>("subregion1")->create_region("subsubregion1");
  region2->get_component<CRegion>("subregion1")->create_region("subsubregion2");

  CRegion_iterator it = region2->begin();
  for ( ; it != region2->end() ; ++it) {
    CFinfo << "path = " << it->full_path().string() << " \n" << CFendl;
  }
  
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CTableTest )
{
  // CFinfo << "testing CTable \n" << CFendl;

  // Create mesh component
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> mesh ( new CMesh  ( "mesh" ) );

  root->add_component( mesh );
  
  // Create one region inside mesh
  boost::shared_ptr<CMesh> p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  p_mesh->create_region("region");
  SafePtr<CRegion> region = p_mesh->get_component<CRegion>("region");

  // Create connectivity table inside the region
  region->create_connectivityTable("connTable");
  SafePtr<CTable> connTable = region->get_component<CTable>("connTable");
  
  // check constructor
  BOOST_CHECK_EQUAL(connTable->get_table().size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable->get_table().shape()[1],(Uint) 0);
  BOOST_CHECK_EQUAL(connTable->get_table().num_elements(),(Uint) 0);
  
  // check initalization
  Uint nbCols = 5;
  connTable->initialize(nbCols);
  Buffer<CTable::ConnectivityTable> tableBuffer = connTable->create_buffer();
  
  BOOST_CHECK_EQUAL(connTable->get_table().size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable->get_table().shape()[1],(Uint) 5);
  BOOST_CHECK_EQUAL(connTable->get_table().num_elements(),(Uint) 0);  
  
  // check for adding rows to table
  std::vector<Uint> row(nbCols);
  for (Uint i=0; i<nbCols; ++i)
    row[i] = i;
    
  tableBuffer.add_row(row);
  tableBuffer.flush();
  BOOST_CHECK_EQUAL(connTable->get_table().size(),(Uint) 1);
  BOOST_CHECK_EQUAL(connTable->get_table().shape()[1],(Uint) 5);
  BOOST_CHECK_EQUAL(connTable->get_table().num_elements(),(Uint) 5);  
  
  
  // check if buffer flushes without calling flush by the user
  for (Uint i=0; i<1023; ++i)
    tableBuffer.add_row(row);
  BOOST_CHECK_EQUAL(connTable->get_table().size(),(Uint) 1);
  BOOST_CHECK_EQUAL(connTable->get_table().shape()[1],(Uint) 5);
  BOOST_CHECK_EQUAL(connTable->get_table().num_elements(),(Uint) 5); 
  
  tableBuffer.add_row(row);
  BOOST_CHECK_EQUAL(connTable->get_table().size(),(Uint) 1025);
  BOOST_CHECK_EQUAL(connTable->get_table().shape()[1],(Uint) 5);
  BOOST_CHECK_EQUAL(connTable->get_table().num_elements(),(Uint) 5*1025); 
      
  // check if accessor / mutator works
  BOOST_CHECK_EQUAL(connTable->get_table()[0][0], (Uint) 0);
  BOOST_CHECK_EQUAL(connTable->get_table()[1][1], (Uint) 1);
  BOOST_CHECK_EQUAL(connTable->get_table()[2][2], (Uint) 2);
  
  // check if a row can be accessed
  CTable::Row rowRef = connTable->get_table()[35];
  for (Uint i=0; i<nbCols; ++i)
    BOOST_CHECK_EQUAL(rowRef[i], i);
  
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CArrayTest )
{
  // Create a CElements component
  boost::shared_ptr<CArray> coordinates (new CArray("coords")) ;

  // initialize the array
  Uint dim = 2;
  coordinates->initialize(dim);
  Buffer<CArray::Array> coordinatesBuffer = coordinates->create_buffer();
  
  // Add coordinates to the array
  coordinatesBuffer.add_row(create_coord( 0.0 , 0.0 ));
  coordinatesBuffer.add_row(create_coord( 1.0 , 0.0 ));
  coordinatesBuffer.add_row(create_coord( 1.0 , 1.0 ));
  coordinatesBuffer.add_row(create_coord( 0.0 , 1.0 ));
  coordinatesBuffer.flush();
  
  BOOST_CHECK_EQUAL(coordinates->get_array()[2][1], 1.0);  
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CElementsTriag2DTest )
{
  // Create a CElements component
  boost::shared_ptr<CElements> comp (new CElements("comp")) ;

  // The element is automatically triangle for now
  comp->set_elementType("Triag2D");
  BOOST_CHECK_EQUAL(comp->get_elementType()->getShapeName(), "Triag");
  BOOST_CHECK_EQUAL(comp->get_elementType()->getNbFaces(), (Uint) 3);

  // Check volume calculation
  RealVector A(2), B(2), C(2);
  std::vector<RealVector*> coord(3);
  A[XX]=15; A[YY]=15;   coord[0] = &A;
  B[XX]=40; B[YY]=25;   coord[1] = &B;
  C[XX]=25; C[YY]=30;   coord[2] = &C;
  BOOST_CHECK_EQUAL(comp->get_elementType()->computeVolume(coord), 137.5);
  
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CElementsQuad2DTest )
{
  // Create a CElements component
  boost::shared_ptr<CElements> comp (new CElements("comp")) ;

  // The element is automatically triangle for now
  comp->set_elementType("Quad2D");
  BOOST_CHECK_EQUAL(comp->get_elementType()->getShapeName(), "Quad");
  BOOST_CHECK_EQUAL(comp->get_elementType()->getNbFaces(), (Uint) 4);

  // Check volume calculation
  RealVector A(2), B(2), C(2), D(2);
  std::vector<RealVector*> coord(4);
  A[XX]=15; A[YY]=15;   coord[0] = &A;
  B[XX]=40; B[YY]=25;   coord[1] = &B;
  C[XX]=25; C[YY]=30;   coord[2] = &C;
  D[XX]=30; D[YY]=40;   coord[3] = &D;
  BOOST_CHECK_EQUAL(comp->get_elementType()->computeVolume(coord), 150);
  
}

BOOST_AUTO_TEST_CASE( CArrayTemplates )
{
  CArray vectorArray("vector");
  vectorArray.initialize(3);
  //CFinfo << "numdim = " << CArray<VECTOR>::Array::NumDims() << "\n" << CFendl;

  // CArray<SCALAR> scalarArray("scalar");
  // scalarArray.initialize(3);
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

