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
  CMesh::Ptr p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  CRegion::Ptr region1 = p_mesh->create_region("region1");
  BOOST_CHECK_EQUAL ( region1->full_path().string() , "//root/mesh/region1" );

  // Create second region inside mesh, with 2 subregions inside
  CRegion::Ptr region2 = p_mesh->create_region("region2");
  region2->create_region("subregion1");
  CRegion::Ptr subregion = region2->create_region("subregion2");
  BOOST_CHECK_EQUAL ( subregion->full_path().string() , "//root/mesh/region2/subregion2" );

  // Create a connectivity table inside a subregion
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
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( AddRemoveTest )
{
  // create table
  CTable::Ptr table (new CTable("table"));
  // initialize with number of columns
  Uint nbCols = 3;
  table->initialize(nbCols);
  // create a buffer to interact with the table
  CTable::Buffer buffer = table->create_buffer();
  
  // make a row
  std::vector<Uint> row(nbCols);
  
  // add 4 rows to buffer
  for(Uint i=0; i<nbCols; i++) row[i] = 0;
  buffer.add_row(row);
  for(Uint i=0; i<nbCols; i++) row[i] = 1;
  buffer.add_row(row);
  for(Uint i=0; i<nbCols; i++) row[i] = 2;
  buffer.add_row(row);
  for(Uint i=0; i<nbCols; i++) row[i] = 3;
  buffer.add_row(row);
  
  // remove row 0 and 2
  buffer.rm_row(0);
  buffer.rm_row(2);
  
  // table should still be empty as the buffer is not flushed
  BOOST_CHECK_EQUAL(table->get_table().size(),(Uint) 0);


  buffer.flush();
  
  // table should have 2 elements as the buffer is flushed
  BOOST_CHECK_EQUAL(table->get_table().size(),(Uint) 2);
  BOOST_CHECK_EQUAL(table->get_table()[0][0], (Uint) 1);
  BOOST_CHECK_EQUAL(table->get_table()[1][0], (Uint) 3);
        
  
  // Test now if 2 rows can be deleted and only 1 added
  
  for(Uint i=0; i<nbCols; i++) row[i] = 4;
  buffer.add_row(row);
  buffer.rm_row(0);
  buffer.rm_row(1);
  
  buffer.flush();
  BOOST_CHECK_EQUAL(table->get_table().size(),(Uint) 2);
  BOOST_CHECK_EQUAL(table->get_table()[1][0], (Uint) 4);
  
  // table row 0 should be unmodified but also marked as empty
  BOOST_CHECK_EQUAL(table->get_table()[0][0], (Uint) 1);
  BOOST_CHECK_EQUAL(buffer.m_nbEmptyArrayRows, (Uint) 1);
  BOOST_CHECK_EQUAL(buffer.m_emptyArrayRows[0], (Uint) 0);
    
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( FlushTest )
{
  // create table
  CTable::Ptr table (new CTable("table"));
  // initialize with number of columns
  Uint nbCols = 3;
  table->initialize(nbCols);
  // create a buffer to interact with the table with buffersize 3 (if no argument, use default buffersize)
  CTable::Buffer buffer = table->create_buffer(3);
  
  // make a row
  std::vector<Uint> row(nbCols);
  
  // add 4 rows to buffer
  for(Uint i=0; i<nbCols; i++) row[i] = 0;
  buffer.add_row(row);
  for(Uint i=0; i<nbCols; i++) row[i] = 1;
  buffer.add_row(row);
  for(Uint i=0; i<nbCols; i++) row[i] = 2;
  buffer.add_row(row);
  
  // buffer should automatically flush as its size is 3
  BOOST_CHECK_EQUAL(table->get_table().size(),(Uint) 3);
  
  // buffer is now empty.
  // add a row to buffer, remove that same row, and add another row.
  for(Uint i=0; i<nbCols; i++) row[i] = 4;
  buffer.add_row(row);
  buffer.rm_row(3);
  for(Uint i=0; i<nbCols; i++) row[i] = 3;
  buffer.add_row(row);
  
  // the buffer should have one filled in value. flush it
  buffer.flush();
  
  // the table should have grown with 1 as the buffer with 1 item was flushed
  BOOST_CHECK_EQUAL(table->get_table().size(),(Uint) 4);
  BOOST_CHECK_EQUAL(table->get_table()[3][0],(Uint) 3);
  
  // remove row 0, 1, 2
  buffer.rm_row(0);
  buffer.rm_row(1);
  buffer.rm_row(2);
  
  // table still has 4 rows, but first 3 rows are marked as disabled
  BOOST_CHECK_EQUAL(table->get_table().size(),(Uint) 4);
  
  buffer.compact();
  // now the table should only have 1 row, as the 3 disabled rows should be removed
  
  BOOST_CHECK_EQUAL(table->get_table().size(),(Uint) 1);
  BOOST_CHECK_EQUAL(table->get_table()[0][0],(Uint) 3);
  
  

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CTableTest )
{
  // CFinfo << "testing CTable \n" << CFendl;
  Logger::getInstance().getStream(Logger::DEBUG).setLogLevel(SILENT);
  // Create mesh component
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> mesh ( new CMesh  ( "mesh" ) );

  root->add_component( mesh );
  
  // Create one region inside mesh
  CMesh::Ptr p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  CRegion::Ptr region = p_mesh->create_region("region");

  // Create connectivity table inside the region
  region->create_connectivityTable("connTable");
  CTable::Ptr connTable = region->get_component<CTable>("connTable");
  
  // check constructor
  BOOST_CHECK_EQUAL(connTable->get_table().size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable->get_table().shape()[1],(Uint) 0);
  BOOST_CHECK_EQUAL(connTable->get_table().num_elements(),(Uint) 0);
  
  // check initalization
  Uint nbCols = 5;
  connTable->initialize(nbCols);
  CTable::Buffer tableBuffer = connTable->create_buffer();
  
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
  CArray::Ptr coordinates (new CArray("coords")) ;

  // initialize the array
  Uint dim = 2;
  coordinates->initialize(dim);
  CArray::Buffer coordinatesBuffer = coordinates->create_buffer();
  
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
  CElements::Ptr comp (new CElements("comp")) ;

  // The element is automatically triangle for now
  comp->set_elementType("P1-Triag2D");
  BOOST_CHECK_EQUAL(comp->get_elementType()->getShapeName(), "Triag");
  BOOST_CHECK_EQUAL(comp->get_elementType()->getNbFaces(), (Uint) 3);

  // Check volume calculation
  CArray::Array coord(boost::extents[3][2]);
  coord[0][XX]=15; coord[0][YY]=15;
  coord[1][XX]=40; coord[1][YY]=25;
  coord[2][XX]=25; coord[2][YY]=30;
  std::vector<CArray::Row> coordvec;
  coordvec.reserve(3);
  coordvec.push_back(coord[0]);
  coordvec.push_back(coord[1]);
  coordvec.push_back(coord[2]);
  BOOST_CHECK_EQUAL(comp->get_elementType()->computeVolume(coordvec), 137.5);
  
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CElementsQuad2DTest )
{
  // Create a CElements component
  CElements::Ptr comp (new CElements("comp")) ;

  // The element is automatically triangle for now
  comp->set_elementType("P1-Quad2D");
  BOOST_CHECK_EQUAL(comp->get_elementType()->getShapeName(), "Quad");
  BOOST_CHECK_EQUAL(comp->get_elementType()->getNbFaces(), (Uint) 4);

  // Check volume calculation
  CArray::Array coord(boost::extents[4][2]);
  coord[0][XX]=15; coord[0][YY]=15;
  coord[1][XX]=40; coord[1][YY]=25;
  coord[2][XX]=25; coord[2][YY]=30;
  coord[3][XX]=30; coord[3][YY]=40;
  std::vector<CArray::Row> coordvec;
  coordvec.reserve(4);
  coordvec.push_back(coord[0]);
  coordvec.push_back(coord[1]);
  coordvec.push_back(coord[2]);
  coordvec.push_back(coord[3]);


  BOOST_CHECK_EQUAL(comp->get_elementType()->computeVolume(coordvec), 150);
  
}

BOOST_AUTO_TEST_CASE( CArrayTemplates )
{
  CArray vectorArray("vector");
  vectorArray.initialize(3);
  //CFinfo << "numdim = " << CArray<VECTOR>::Array::NumDims() << "\n" << CFendl;

  // CArray<SCALAR> scalarArray("scalar");
  // scalarArray.initialize(3);
  
}

BOOST_AUTO_TEST_CASE( moving_mesh_components_around )
{
  CRoot::Ptr root = CRoot::create ( "root" );
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  CRegion::Ptr regions = mesh->create_region("regions");

  CRegion::Ptr subregion1 = regions->create_region("subregion1");
  BOOST_CHECK_EQUAL(subregion1->has_component_of_type<CRegion>(),false);

  subregion1->create_connectivityTable("table");
  BOOST_CHECK_EQUAL(subregion1->has_component_of_type<CRegion>(),false);

  std::string type = DEMANGLED_TYPEID(CRegion);
  CFinfo << "type = " << type << "\n" << CFendl;
  CFinfo << "has_tag = " << subregion1->has_tag(CRegion::getClassName()) << "\n" << CFendl;
  // create subregion2 in the wrong place
  CRegion::Ptr subregion2 = subregion1->create_region("subregion2");
  BOOST_CHECK_EQUAL(subregion1->has_component_of_type<CRegion>(),true);
  BOOST_CHECK_EQUAL(regions->get_components_by_tag<CRegion>(CRegion::getClassName()).size(), (Uint) 1);


  // move subregion 2 to the right place
  subregion1->remove_component(subregion2->name());
  regions->add_component(subregion2);
  BOOST_CHECK_EQUAL(subregion1->has_component_of_type<CRegion>(),false);

  BOOST_CHECK_EQUAL(regions->get_components_by_tag<CRegion>(CRegion::getClassName()).size(), (Uint) 2);


}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( tags )
{
  CRoot::Ptr root = CRoot::create ( "root" );
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
  CRegion::Ptr regions = mesh->create_region("regions");

  mesh->add_tag("lolo");
  std::vector<std::string> tags = mesh->get_tags();
  BOOST_CHECK_EQUAL(tags[0],"Component");
  BOOST_CHECK_EQUAL(tags[1],"CMesh");
  BOOST_CHECK_EQUAL(tags[2],"lolo");

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

