#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"
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
  // CFinfo << "testing MeshComponents \n" << CFflush;

  // Create root and mesh component
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> mesh ( new CMesh  ( "mesh" ) );

  root->add_component( mesh );

  BOOST_CHECK_EQUAL ( mesh->name() , "mesh" );
  BOOST_CHECK_EQUAL ( mesh->path().string() , "//root" );
  BOOST_CHECK_EQUAL ( mesh->full_path().string() , "//root/mesh" );
  
  // Create one region inside mesh
  CMesh::Ptr p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  CRegion& region1 = p_mesh->create_region("region1");
  BOOST_CHECK_EQUAL ( region1.full_path().string() , "//root/mesh/region1" );

  // Create second region inside mesh, with 2 subregions inside
  CRegion& region2 = p_mesh->create_region("region2");
  
  CFinfo << p_mesh->tree() << CFendl;
  region2.create_region("subregion1");
  CRegion& subregion = region2.create_region("subregion2");
  BOOST_CHECK_EQUAL ( subregion.full_path().string() , "//root/mesh/base/region2/subregion2" );

  // Create a connectivity table inside a subregion
  subregion.create_component_type<CTable>("connTable");
  BOOST_CHECK_EQUAL ( get_named_component(subregion, "connTable").full_path().string() , "//root/mesh/base/region2/subregion2/connTable" );
  
  // Create a elementsType component inside a subregion
  subregion.create_component_type<CElements>("elementType");
  BOOST_CHECK_EQUAL ( get_named_component(subregion, "elementType").full_path().string() , "//root/mesh/base/region2/subregion2/elementType" );
  
  // Create an array of coordinates inside mesh
  p_mesh->create_component_type<CArray>("coordinates");
  BOOST_CHECK_EQUAL ( get_named_component(*p_mesh, "coordinates").full_path().string() , "//root/mesh/coordinates" );
  
  get_named_component_typed<CRegion>(region2, "subregion1").create_region("subsubregion1");
  get_named_component_typed<CRegion>(region2, "subregion1").create_region("subsubregion2");
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
  BOOST_CHECK_EQUAL(table->array().size(),(Uint) 0);


  buffer.flush();
  
  // table should have 2 elements as the buffer is flushed
  BOOST_CHECK_EQUAL(table->array().size(),(Uint) 2);
  BOOST_CHECK_EQUAL(table->array()[0][0], (Uint) 1);
  BOOST_CHECK_EQUAL(table->array()[1][0], (Uint) 3);
        
  
  // Test now if 2 rows can be deleted and only 1 added
  
  for(Uint i=0; i<nbCols; i++) row[i] = 4;
  buffer.add_row(row);
  buffer.rm_row(0);
  buffer.rm_row(1);
  BOOST_CHECK_EQUAL(table->array().size(),(Uint) 2);

  
  buffer.flush();
  BOOST_CHECK_EQUAL(table->array().size(),(Uint) 1);
  BOOST_CHECK_EQUAL(table->array()[0][0], (Uint) 4);
    
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
  BOOST_CHECK_EQUAL(buffer.total_allocated(), (Uint) 3);
  for(Uint i=0; i<nbCols; i++) row[i] = 3;
  buffer.add_row(row);
  // adding that last row allocated a new buffer of size 3
  BOOST_CHECK_EQUAL(buffer.total_allocated(), (Uint) 6);
  buffer.flush();
  // the flush copied everything in the table, and removed the buffers
  BOOST_CHECK_EQUAL(buffer.total_allocated(), (Uint) 4);
  BOOST_CHECK_EQUAL(table->array().size(),(Uint) 4);
  
  // buffer is now empty.
  // add a row to buffer, remove that same row, and add another row.
  for(Uint i=0; i<nbCols; i++) row[i] = 4;
  buffer.add_row(row);
  buffer.rm_row(4);
  for(Uint i=0; i<nbCols; i++) row[i] = 5;
  buffer.add_row(row);
  BOOST_CHECK_EQUAL(buffer.total_allocated(), (Uint) 7);

  buffer.flush();
  // the table should have grown with 1 as the buffer with 1 item was flushed
  BOOST_CHECK_EQUAL(buffer.total_allocated(), (Uint) 5);
  BOOST_CHECK_EQUAL(table->array().size(),(Uint) 5);
  BOOST_CHECK_EQUAL(table->array()[4][0],(Uint) 5);
  
  // remove row 0, 1, 2
  buffer.rm_row(0);
  buffer.rm_row(1);
  buffer.rm_row(2);
  
  // table still has 5 rows, but first 3 rows are marked as disabled
  BOOST_CHECK_EQUAL(table->array().size(),(Uint) 5);
  
  buffer.flush();
  // now the table should only have 2 row, as the 3 disabled rows should be removed
  
  BOOST_CHECK_EQUAL(table->array().size(),(Uint) 2);
  BOOST_CHECK_EQUAL(table->array()[0][0],(Uint) 3);
  BOOST_CHECK_EQUAL(table->array()[1][0],(Uint) 5);

  
  

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CTableTest )
{
  // CFinfo << "testing CTable \n" << CFflush;
  Logger::instance().getStream(Logger::DEBUG).setLogLevel(SILENT);
  // Create mesh component
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> mesh ( new CMesh  ( "mesh" ) );

  root->add_component( mesh );
  
  // Create one region inside mesh
  CMesh::Ptr p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  CRegion& region = p_mesh->create_region("region");

  // Create connectivity table inside the region
  CTable& connTable = *region.create_component_type<CTable>("connTable");
  
  // check constructor
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.array().shape()[1],(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 0);
  
  // check initalization
  Uint nbCols = 5;
  connTable.initialize(nbCols);
  CTable::Buffer tableBuffer = connTable.create_buffer();
  
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.array().shape()[1],(Uint) 5);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 0);  
  
  // check for adding rows to table
  std::vector<Uint> row(nbCols);
  for (Uint i=0; i<nbCols; ++i)
    row[i] = i;
    
  tableBuffer.add_row(row);
  tableBuffer.flush();
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 1);
  BOOST_CHECK_EQUAL(connTable.array().shape()[1],(Uint) 5);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 5);  
  
  
  // check if buffer flushes without calling flush by the user
  for (Uint i=0; i<1023; ++i)
    tableBuffer.add_row(row);
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 1);
  BOOST_CHECK_EQUAL(connTable.array().shape()[1],(Uint) 5);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 5); 
  
  tableBuffer.add_row(row);
  tableBuffer.flush();
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 1025);
  BOOST_CHECK_EQUAL(connTable.array().shape()[1],(Uint) 5);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 5*1025); 
      
  // check if accessor / mutator works
  BOOST_CHECK_EQUAL(connTable[0][0], (Uint) 0);
  BOOST_CHECK_EQUAL(connTable[1][1], (Uint) 1);
  BOOST_CHECK_EQUAL(connTable[2][2], (Uint) 2);
  
  // check if a row can be accessed
  CTable::Row rowRef = connTable[35];
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
  
  BOOST_CHECK_EQUAL(coordinates->array()[2][1], 1.0);  
}

//////////////////////////////////////////////////////////////////////////////



BOOST_AUTO_TEST_CASE( CArrayTemplates )
{
  CArray vectorArray("vector");
  vectorArray.initialize(3);
  //CFinfo << "numdim = " << CArray<VECTOR>::Array::NumDims() << "\n" << CFflush;

  // CArray<SCALAR> scalarArray("scalar");
  // scalarArray.initialize(3);
  
}

BOOST_AUTO_TEST_CASE( moving_mesh_components_around )
{
  CRoot::Ptr root = CRoot::create ( "root" );
  CMesh::Ptr mesh = root->create_component_type<CMesh>("mesh");
  CRegion& regions = mesh->create_region("regions");

  CRegion& subregion1 = regions.create_region("subregion1");
  BOOST_CHECK_EQUAL(range_typed<CRegion>(subregion1).empty(),true);

  subregion1.create_component_type<CTable>("table");
  BOOST_CHECK_EQUAL(range_typed<CRegion>(subregion1).empty(),true);

  // create subregion2 in the wrong place
  CRegion& subregion2 = subregion1.create_region("subregion2");
  BOOST_CHECK_EQUAL(range_typed<CRegion>(subregion1).empty(),false);
  BOOST_CHECK_EQUAL(count(range_typed<CRegion>(regions)), (Uint) 1);


  // move subregion 2 to the right place
  Component::Ptr subregion2_ptr = subregion1.remove_component(subregion2.name());
  regions.add_component(subregion2_ptr);
  BOOST_CHECK_EQUAL(range_typed<CRegion>(subregion1).empty(),true);
  BOOST_CHECK_EQUAL(count(range_typed<CRegion>(regions)), (Uint) 2);


}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

