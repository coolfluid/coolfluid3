// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh component classes"

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CDynTable.hpp"
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

  boost::shared_ptr<Component> mesh ( allocate_component_type<CMesh>  ( "mesh" ) );

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
  subregion.create_component_type<CTable<Uint> >("connTable");
  BOOST_CHECK_EQUAL ( get_named_component(subregion, "connTable").full_path().string() , "//root/mesh/base/region2/subregion2/connTable" );
  
  // Create a elementsType component inside a subregion
  subregion.create_component_type<CElements>("elementType");
  BOOST_CHECK_EQUAL ( get_named_component(subregion, "elementType").full_path().string() , "//root/mesh/base/region2/subregion2/elementType" );
  
  // Create an array of coordinates inside mesh
  p_mesh->create_component_type<CTable<Real> >("coordinates");
  BOOST_CHECK_EQUAL ( get_named_component(*p_mesh, "coordinates").full_path().string() , "//root/mesh/coordinates" );
  
  get_named_component_typed<CRegion>(region2, "subregion1").create_region("subsubregion1");
  get_named_component_typed<CRegion>(region2, "subregion1").create_region("subsubregion2");
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( AddRemoveTest )
{
  // create table
  CTable<Uint>::Ptr table (new CTable<Uint>("table"));
  // initialize with number of columns
  Uint nbCols = 3;
  table->initialize(nbCols);
  // create a buffer to interact with the table
  CTable<Uint>::Buffer buffer = table->create_buffer();
  
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
  CTable<Uint>::Ptr table (new CTable<Uint>("table"));
  // initialize with number of columns
  Uint nbCols = 3;
  table->initialize(nbCols);
  // create a buffer to interact with the table with buffersize 3 (if no argument, use default buffersize)
  CTable<Uint>::Buffer buffer = table->create_buffer(3);
  
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
  
	boost::shared_ptr< std::vector<Uint> > old_indexes_ptr = buffer.flush();
	std::vector<Uint>& old_indexes = *old_indexes_ptr;
  // now the table should only have 2 row, as the 3 disabled rows should be removed
  
  BOOST_CHECK_EQUAL(table->array().size(),(Uint) 2);
  BOOST_CHECK_EQUAL(table->array()[0][0],(Uint) 3);
  BOOST_CHECK_EQUAL(table->array()[1][0],(Uint) 5);
	
	// check old indices
	BOOST_CHECK_EQUAL(old_indexes[0] , (Uint) 3 );
	BOOST_CHECK_EQUAL(old_indexes[1] , (Uint) 4 );

  
  

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CTable_Uint_Test )
{
  // CFinfo << "testing CTable<Uint> \n" << CFflush;
  Logger::instance().getStream(Logger::DEBUG).setLogLevel(SILENT);
  // Create mesh component
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> mesh ( allocate_component_type<CMesh>  ( "mesh" ) );

  root->add_component( mesh );
  
  // Create one region inside mesh
  CMesh::Ptr p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  CRegion& region = p_mesh->create_region("region");

  // Create connectivity table inside the region
  CTable<Uint>& connTable = *region.create_component_type<CTable<Uint> >("connTable");
  
  // check constructor
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.row_size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 0);
  
  // check initalization
  Uint nbCols = 5;
  connTable.initialize(nbCols);
  CTable<Uint>::Buffer tableBuffer = connTable.create_buffer();
  
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.row_size(),(Uint) 5);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 0);  
  
  // check for adding rows to table
  std::vector<Uint> row(nbCols);
  for (Uint i=0; i<nbCols; ++i)
    row[i] = i;
    
  tableBuffer.add_row(row);
  tableBuffer.flush();
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 1);
  BOOST_CHECK_EQUAL(connTable.row_size(),(Uint) 5);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 5);  
  
  
  // check if buffer flushes without calling flush by the user
  for (Uint i=0; i<1023; ++i)
    tableBuffer.add_row(row);
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 1);
  BOOST_CHECK_EQUAL(connTable.row_size(),(Uint) 5);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 5); 
  
  tableBuffer.add_row(row);
  tableBuffer.flush();
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 1025);
  BOOST_CHECK_EQUAL(connTable.row_size(),(Uint) 5);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 5*1025); 
      
  // check if accessor / mutator works
  BOOST_CHECK_EQUAL(connTable[0][0], (Uint) 0);
  BOOST_CHECK_EQUAL(connTable[1][1], (Uint) 1);
  BOOST_CHECK_EQUAL(connTable[2][2], (Uint) 2);
  
  // check if a row can be accessed
  CTable<Uint>::Row rowRef = connTable[35];
  for (Uint i=0; i<nbCols; ++i)
    BOOST_CHECK_EQUAL(rowRef[i], i);
  
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CTable_Real_Test )
{
  // Create a CElements component
  CTable<Real>::Ptr coordinates (new CTable<Real>("coords")) ;

  // initialize the array
  Uint dim = 2;
  coordinates->initialize(dim);
	BOOST_CHECK_EQUAL(coordinates->row_size(),dim);
  CTable<Real>::Buffer coordinatesBuffer = coordinates->create_buffer();
  
  // Add coordinates to the array
  coordinatesBuffer.add_row(create_coord( 0.0 , 0.0 ));
  coordinatesBuffer.add_row(create_coord( 1.0 , 0.0 ));
  coordinatesBuffer.add_row(create_coord( 1.0 , 1.0 ));
  coordinatesBuffer.add_row(create_coord( 0.0 , 1.0 ));
  coordinatesBuffer.flush();
  
  BOOST_CHECK_EQUAL(coordinates->array()[2][1], 1.0);  
}

//////////////////////////////////////////////////////////////////////////////



BOOST_AUTO_TEST_CASE( CTable_Real_Templates )
{
  CTable<Real> vectorArray("vector");
  vectorArray.initialize(3);
  //CFinfo << "numdim = " << CTable<Real><VECTOR>::Array::NumDims() << "\n" << CFflush;

  // CTable<Real><SCALAR> scalarArray("scalar");
  // scalarArray.initialize(3);
  
}

BOOST_AUTO_TEST_CASE( moving_mesh_components_around )
{
  CRoot::Ptr root = CRoot::create ( "root" );
  CMesh::Ptr mesh = root->create_component_type<CMesh>("mesh");
  CRegion& regions = mesh->create_region("regions");

  CRegion& subregion1 = regions.create_region("subregion1");
  BOOST_CHECK_EQUAL(range_typed<CRegion>(subregion1).empty(),true);

  subregion1.create_component_type<CTable<Uint> >("table");
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

BOOST_AUTO_TEST_CASE( CList_tests )
{
	CList<bool> bool_list ("bool_list");
	BOOST_CHECK_EQUAL(bool_list.type_name(),"CList<bool>");
	
	CList<int>::Ptr integer_list (new CList<int>("integer_list"));
	BOOST_CHECK_EQUAL(integer_list->type_name(),"CList<integer>");

	CList<Uint>::Ptr unsigned_list (new CList<Uint>("unsigned_list"));
	BOOST_CHECK_EQUAL(unsigned_list->type_name(),"CList<unsigned>");
	
	CList<Real>::Ptr real_list (new CList<Real>("real_list"));
	BOOST_CHECK_EQUAL(real_list->type_name(),"CList<real>");
	
	CList<std::string>::Ptr string_list (new CList<std::string>("string_list"));
	BOOST_CHECK_EQUAL(string_list->type_name(),"CList<string>");
	
	bool_list.resize(10);
	BOOST_CHECK_EQUAL(bool_list.size(),(Uint) 10);
	bool_list[0] = true;
	bool_list[5] = true;
	
	for (Uint i=0; i<10; ++i)
	{
		switch (i)
		{
			case 0:
			case 5:
				BOOST_CHECK_EQUAL(bool_list[i],true);
				break;
			default:
				BOOST_CHECK_EQUAL(bool_list[i],false);
				break;
		}
	}
	
	bool_list.resize(20);
	BOOST_CHECK_EQUAL(bool_list.size(),(Uint) 20);

	for (Uint i=0; i<20; ++i)
	{
		switch (i)
		{
			case 0:
			case 5:
				BOOST_CHECK_EQUAL(bool_list[i],true);
				break;
			default:
				BOOST_CHECK_EQUAL(bool_list[i],false);
				break;
		}
	}
}

BOOST_AUTO_TEST_CASE( ListAddRemoveTest )
{
  // create table
  CTable<Uint>::Ptr table (new CTable<Uint>("table"));
  // initialize with number of columns
  Uint nbCols = 3;
  table->initialize(nbCols);
  // create a buffer to interact with the table
  CTable<Uint>::Buffer buffer = table->create_buffer();
  
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

BOOST_AUTO_TEST_CASE( ListFlushTest )
{
  // create table
  CList<Uint>list ("list");
  // create a buffer to interact with the list with buffersize 3 (if no argument, use default buffersize)
  CList<Uint>::Buffer buffer = list.create_buffer(3);
  
  // make a row
  Uint row;
  
  // add 4 rows to buffer
  row = 0;
  buffer.add_row(row);
  row = 1;
  buffer.add_row(row);
	row = 2;
	buffer.add_row(row);
	BOOST_CHECK_EQUAL(buffer.total_allocated(), (Uint) 3);
	row = 3;
	buffer.add_row(row);

	// adding that last row allocated a new buffer of size 3
  BOOST_CHECK_EQUAL(buffer.total_allocated(), (Uint) 6);
  buffer.flush();
  // the flush copied everything in the table, and removed the buffers
  BOOST_CHECK_EQUAL(buffer.total_allocated(), (Uint) 4);
  BOOST_CHECK_EQUAL(list.size(),(Uint) 4);
	
  
  // buffer is now empty.
  // add a row to buffer, remove that same row, and add another row.
	row = 4;
  buffer.add_row(row);
  buffer.rm_row(4);
	row = 5;
  buffer.add_row(row);
  BOOST_CHECK_EQUAL(buffer.total_allocated(), (Uint) 7);
	
  buffer.flush();
  // the table should have grown with 1 as the buffer with 1 item was flushed
  BOOST_CHECK_EQUAL(buffer.total_allocated(), (Uint) 5);
  BOOST_CHECK_EQUAL(list.size(),(Uint) 5);
  BOOST_CHECK_EQUAL(list[4],(Uint) 5);
  
  // remove row 0, 1, 2
  buffer.rm_row(0);
  buffer.rm_row(1);
  buffer.rm_row(2);
  
  // table still has 5 rows, but first 3 rows are marked as disabled
  BOOST_CHECK_EQUAL(list.size(),(Uint) 5);
  
  buffer.flush();
  // now the table should only have 2 row, as the 3 disabled rows should be removed
  
  BOOST_CHECK_EQUAL(list.size(),(Uint) 2);
  BOOST_CHECK_EQUAL(list[0],(Uint) 3);
  BOOST_CHECK_EQUAL(list[1],(Uint) 5);
	
  
}

BOOST_AUTO_TEST_CASE ( CDynTable_test )
{
  CDynTable<Uint> table ("table");
  CDynTable<Uint>::Buffer buffer = table.create_buffer();
  
  std::vector<Uint> row;
  
  row.resize(3);
  for(Uint i=0; i<row.size(); i++) row[i] = 0;
  buffer.add_row(row);

  row.resize(4);
  for(Uint i=0; i<row.size(); i++) row[i] = 1;
  buffer.add_row(row);
  
  row.resize(6);
  for(Uint i=0; i<row.size(); i++) row[i] = 2;
  buffer.add_row(row);
  
  row.resize(2);
  for(Uint i=0; i<row.size(); i++) row[i] = 3;
  buffer.add_row(row);
  
  BOOST_CHECK_EQUAL(table.size(), (Uint) 0);
  
  buffer.flush();
  
  BOOST_CHECK_EQUAL(table.size(), (Uint) 4);
  
  BOOST_CHECK_EQUAL(table[0][0], (Uint) 0);
  BOOST_CHECK_EQUAL(table[0].size(), (Uint) 3);
  BOOST_CHECK_EQUAL(table.row_size(0), (Uint) 3);
  
  BOOST_CHECK_EQUAL(table[1][0], (Uint) 1);
  BOOST_CHECK_EQUAL(table[1].size(), (Uint) 4);
  BOOST_CHECK_EQUAL(table.row_size(1), (Uint) 4);
  
  
  BOOST_CHECK_EQUAL(table[2][0], (Uint) 2);
  BOOST_CHECK_EQUAL(table[2].size(), (Uint) 6);
  BOOST_CHECK_EQUAL(table.row_size(2), (Uint) 6);
  
  
  BOOST_CHECK_EQUAL(table[3][0], (Uint) 3);
  BOOST_CHECK_EQUAL(table[3].size(), (Uint) 2);
  BOOST_CHECK_EQUAL(table.row_size(3), (Uint) 2);
  

   // buffer is now empty.
   // add a row to buffer, remove that same row, and add another row.
  row.resize(4);
  for(Uint i=0; i<row.size(); i++) row[i] = 4;
  buffer.add_row(row);
  buffer.rm_row(4);
  
  row.resize(7);
  for(Uint i=0; i<row.size(); i++) row[i] = 5;
  buffer.add_row(row);
  
  buffer.flush();

  // the table should have grown with 1 as the buffer with 1 item was flushed
  BOOST_CHECK_EQUAL(table.size(),(Uint) 5);
  BOOST_CHECK_EQUAL(table[4][0],(Uint) 5);
  BOOST_CHECK_EQUAL(table.row_size(4), (Uint) 7);

  // remove row 0, 1, 2
  buffer.rm_row(0);
  buffer.rm_row(1);
  buffer.rm_row(2);



  // table still has 5 rows, but first 3 rows are marked as disabled
  BOOST_CHECK_EQUAL(table.size(),(Uint) 5);

  // Flush the buffer
 	boost::shared_ptr< std::vector<Uint> > old_indexes_ptr = buffer.flush();
 	std::vector<Uint>& old_indexes = *old_indexes_ptr;
  
  // now the table should only have 2 row, as the 3 disabled rows should be removed

  BOOST_CHECK_EQUAL(table.size(),(Uint) 2);
  
  BOOST_CHECK_EQUAL(table[0][0],(Uint) 3);
  BOOST_CHECK_EQUAL(table.row_size(0), (Uint) 2);
  
  BOOST_CHECK_EQUAL(table[1][0],(Uint) 5);
  BOOST_CHECK_EQUAL(table.row_size(1), (Uint) 7);
  

 	// check old indices
 	BOOST_CHECK_EQUAL(old_indexes[0] , (Uint) 3 );
 	BOOST_CHECK_EQUAL(old_indexes[1] , (Uint) 4 );
  
  
  
  
}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

