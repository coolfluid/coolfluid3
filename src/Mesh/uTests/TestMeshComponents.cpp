#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
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

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshComponent_TestSuite, MeshComponent_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( MeshComponentTest )
{
  // CFinfo << "testing MeshComponents \n" << CFendl;

  // Create root and mesh component
  boost::shared_ptr<Component> mesh ( new CMesh  ( "mesh" ) );
  boost::shared_ptr<Component> root ( new CRoot  ( "root" ) );

  root->add_component( mesh );

  BOOST_CHECK_EQUAL ( mesh->name() , "mesh" );
  BOOST_CHECK_EQUAL ( mesh->path().string() , "//root" );
  BOOST_CHECK_EQUAL ( mesh->full_path().string() , "//root/mesh" );
  
  // Create one region inside mesh
  boost::shared_ptr<CMesh> p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  p_mesh->create_region("region1");
  SafePtr<CRegion> region1 = p_mesh->get_component("region1").d_castTo<CRegion>();
  BOOST_CHECK_EQUAL ( region1->full_path().string() , "//root/mesh/region1" );

  // Create second region inside mesh, with 2 subregions inside
  p_mesh->create_region("region2");
  SafePtr<CRegion> region2 = p_mesh->get_component("region2").d_castTo<CRegion>();
  region2->create_region("subregion1");
  region2->create_region("subregion2");
  BOOST_CHECK_EQUAL ( region2->get_component("subregion2")->full_path().string() , "//root/mesh/region2/subregion2" );

  // Create a connectivity table inside a subregion
  SafePtr<CRegion> subregion = region2->get_component("subregion2").d_castTo<CRegion>();
  subregion->create_connectivityTable("connTable");
  BOOST_CHECK_EQUAL ( subregion->get_component("connTable")->full_path().string() , "//root/mesh/region2/subregion2/connTable" );
  
  // Create a elementsType component inside a subregion
  subregion->create_elementType("elementType");
  BOOST_CHECK_EQUAL ( subregion->get_component("elementType")->full_path().string() , "//root/mesh/region2/subregion2/elementType" );
  
  
  // Create an array of coordinates inside mesh
  p_mesh->create_array("coordinates");
  BOOST_CHECK_EQUAL ( p_mesh->get_component("coordinates")->full_path().string() , "//root/mesh/coordinates" );
  
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CTableTest )
{
  // CFinfo << "testing CTable \n" << CFendl;

  // Create mesh component
  boost::shared_ptr<Component> mesh ( new CMesh  ( "mesh" ) );
  boost::shared_ptr<Component> root ( new CRoot  ( "root" ) );

  root->add_component( mesh );
  
  // Create one region inside mesh
  boost::shared_ptr<CMesh> p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  p_mesh->create_region("region");
  SafePtr<CRegion> region = p_mesh->get_component("region").d_castTo<CRegion>();

  // Create connectivity table inside the region
  region->create_connectivityTable("connTable");
  SafePtr<CTable> connTable = region->get_component("connTable").d_castTo<CTable>();
  
  // check constructor
  BOOST_CHECK_EQUAL(connTable->nbRows(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable->nbCols(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable->size(),(Uint) 0);
  
  // check initalization
  Uint cols = 5;
  Uint buffersize = 3;
  connTable->initialize(cols,buffersize);
  
  BOOST_CHECK_EQUAL(connTable->nbRows(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable->nbCols(),(Uint) 5);
  BOOST_CHECK_EQUAL(connTable->size(),(Uint) 0);
  
  // check for adding rows to table
  std::vector<Uint> row(cols);
  for (Uint i=0; i<cols; ++i)
    row[i] = i;
    
  connTable->add_row(row);
  connTable->flush();
  BOOST_CHECK_EQUAL(connTable->nbRows(),(Uint) 1);
  BOOST_CHECK_EQUAL(connTable->nbCols(),(Uint) 5);
  BOOST_CHECK_EQUAL(connTable->size(),(Uint) 5);
  
  // check if buffer flushes without calling flush by the user
  for (Uint i=0; i<buffersize; ++i)
    connTable->add_row(row);
  BOOST_CHECK_EQUAL(connTable->nbRows(),(Uint) 4);
  BOOST_CHECK_EQUAL(connTable->nbCols(),(Uint) 5);
  BOOST_CHECK_EQUAL(connTable->size(),(Uint) 20);
  
  // check if accessor / mutator works
  BOOST_CHECK_EQUAL((*connTable)(0,0), (Uint) 0);
  BOOST_CHECK_EQUAL((*connTable)(1,1), (Uint) 1);
  BOOST_CHECK_EQUAL((*connTable)(2,2), (Uint) 2);
  for (Uint i=0; i<cols; ++i) {
    (*connTable)(2,i) = 0;
    BOOST_CHECK_EQUAL((*connTable)(2,i), (Uint) 0);
  }
  
  // check if a row can be set
  std::vector<Uint> row2(cols);
  connTable->set_row(3,row2);
  for (Uint i=0; i<cols; ++i)
    BOOST_CHECK_EQUAL(row2[i], i);
    
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

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

