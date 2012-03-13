// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh component classes"

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Group.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "common/Table.hpp"
#include "common/List.hpp"
#include "common/Table.hpp"
#include "common/DynTable.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"

using namespace boost;
using namespace boost::assign;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct MeshComponent_Fixture
{
  /// common setup for each test case
  MeshComponent_Fixture() :
    root(Core::instance().root())
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
  Component& root;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshComponent_TestSuite, MeshComponent_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( MeshComponentTest )
{
  // CFinfo << "testing MeshComponents \n" << CFflush;

  // Create root and mesh component
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  Mesh& mesh = *root->create_component<Mesh>( "mesh" ) ;

  BOOST_CHECK_EQUAL ( mesh.name() , "mesh" );
  BOOST_CHECK_EQUAL ( mesh.uri().base_path().string() , "cpath:/" );
  BOOST_CHECK_EQUAL ( mesh.uri().string() , "cpath:/mesh" );

  // Create one region inside mesh
  Region& region1 = mesh.topology().create_region("region1");
  BOOST_CHECK_EQUAL ( region1.uri().string() , "cpath:/mesh/topology/region1" );

  // Create second region inside mesh, with 2 subregions inside
  Region& region2 = mesh.topology().create_region("region2");

  CFinfo << mesh.tree() << CFendl;
  region2.create_region("subregion1");
  Region& subregion = region2.create_region("subregion2");
  BOOST_CHECK_EQUAL ( subregion.uri().string() , "cpath:/mesh/topology/region2/subregion2" );

  // Create a connectivity table inside a subregion
  subregion.create_component<Table<Uint> >("connTable");
  BOOST_CHECK_EQUAL ( find_component_with_name(subregion, "connTable").uri().string() , "cpath:/mesh/topology/region2/subregion2/connTable" );

  // Create a elementsType component inside a subregion
  subregion.create_component<Elements>("elementType");
  BOOST_CHECK_EQUAL ( find_component_with_name(subregion, "elementType").uri().string() , "cpath:/mesh/topology/region2/subregion2/elementType" );

  // Create an array of coordinates inside mesh
  mesh.create_component<Table<Real> >("coordinates");
  BOOST_CHECK_EQUAL ( find_component_with_name(mesh, "coordinates").uri().string() , "cpath:/mesh/coordinates" );

  find_component_with_name<Region>(region2, "subregion1").create_region("subsubregion1");
  find_component_with_name<Region>(region2, "subregion1").create_region("subsubregion2");
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( AddRemoveTest )
{
  // create table
  boost::shared_ptr< Table<Uint> > table (allocate_component< Table<Uint> >("table"));
  // initialize with number of columns
  Uint nbCols = 3;
  table->set_row_size(nbCols);
  // create a buffer to interact with the table
  Table<Uint>::Buffer buffer = table->create_buffer();

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

BOOST_AUTO_TEST_CASE( NoChangeFlushTest )
{
  // create table
  boost::shared_ptr< Table<Uint> > table (allocate_component< Table<Uint> >("table"));
  // initialize with number of columns
  Uint nbCols = 2;
  table->set_row_size(nbCols);
  table->resize(6);
  // create a buffer to interact with the table
  Table<Uint>::Buffer buffer = table->create_buffer();
  buffer.flush();
}

BOOST_AUTO_TEST_CASE( FlushTest )
{
  // create table
  boost::shared_ptr< Table<Uint> > table (allocate_component< Table<Uint> >("table"));
  // initialize with number of columns
  Uint nbCols = 3;
  table->set_row_size(nbCols);
  // create a buffer to interact with the table with buffersize 3 (if no argument, use default buffersize)
  Table<Uint>::Buffer buffer = table->create_buffer(3);

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

BOOST_AUTO_TEST_CASE( Table_Uint_Test )
{
  // CFinfo << "testing Table<Uint> \n" << CFflush;
  Logger::instance().getStream(DEBUG).set_log_level(SILENT);
  // Create mesh component
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  Mesh& mesh = *root->create_component<Mesh>  ( "mesh" ) ;

  // Create one region inside mesh
  Region& region = mesh.topology().create_region("region");

  // Create connectivity table inside the region
  Table<Uint>& connTable = *region.create_component<Table<Uint> >("connTable");

  // check constructor
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.row_size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 0);

  // check initalization
  Uint nbCols = 5;
  connTable.set_row_size(nbCols);
  Table<Uint>::Buffer tableBuffer = connTable.create_buffer();

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
  Table<Uint>::Row rowRef = connTable[35];
  for (Uint i=0; i<nbCols; ++i)
    BOOST_CHECK_EQUAL(rowRef[i], i);

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Table_Real_Test )
{
  // Create a Elements component
  boost::shared_ptr< Table<Real> > coordinates (allocate_component< Table<Real> >("coords")) ;

  // initialize the array
  Uint dim = 2;
  coordinates->set_row_size(dim);
  BOOST_CHECK_EQUAL(coordinates->row_size(),dim);
  Table<Real>::Buffer coordinatesBuffer = coordinates->create_buffer();

  // Add coordinates to the array
  coordinatesBuffer.add_row(create_coord( 0.0 , 0.0 ));
  coordinatesBuffer.add_row(create_coord( 1.0 , 0.0 ));
  coordinatesBuffer.add_row(create_coord( 1.0 , 1.0 ));
  coordinatesBuffer.add_row(create_coord( 0.0 , 1.0 ));
  coordinatesBuffer.flush();

  BOOST_CHECK_EQUAL(coordinates->array()[2][1], 1.0);
}

//////////////////////////////////////////////////////////////////////////////



BOOST_AUTO_TEST_CASE( Table_Real_Templates )
{
  Table<Real>& vectorArray = *root.create_component< Table<Real> >("vector");
  vectorArray.set_row_size(3);
  //CFinfo << "numdim = " << Table<Real><VECTOR>::Array::NumDims() << "\n" << CFflush;

  // Table<Real><SCALAR> scalarArray("scalar");
  // scalarArray.initialize(3);

}

BOOST_AUTO_TEST_CASE( moving_mesh_components_around )
{
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));
  Mesh& mesh = *root->create_component<Mesh>("mesh");
  Region& regions = mesh.topology().create_region("regions");

  Region& subregion1 = regions.create_region("subregion1");
  BOOST_CHECK_EQUAL(find_components<Region>(subregion1).empty(),true);

  subregion1.create_component<Table<Uint> >("table");
  BOOST_CHECK_EQUAL(find_components<Region>(subregion1).empty(),true);

  // create subregion2 in the wrong place
  Region& subregion2 = subregion1.create_region("subregion2");
  BOOST_CHECK_EQUAL(find_components<Region>(subregion1).empty(),false);
  BOOST_CHECK_EQUAL(count(find_components<Region>(regions)), (Uint) 1);


  // move subregion 2 to the right place
  boost::shared_ptr<Component> subregion2_ptr = subregion1.remove_component(subregion2.name());
  regions.add_component(subregion2_ptr);
  BOOST_CHECK_EQUAL(find_components<Region>(subregion1).empty(),true);
  BOOST_CHECK_EQUAL(count(find_components<Region>(regions)), (Uint) 2);


}

BOOST_AUTO_TEST_CASE( List_tests )
{
  List<bool>& bool_list = *root.create_component< List<bool> >("bool_list");
  BOOST_CHECK_EQUAL(bool_list.type_name(),"List<bool>");

  boost::shared_ptr< List<int> > integer_list (allocate_component< List<int> >("integer_list"));
  BOOST_CHECK_EQUAL(integer_list->type_name(),"List<integer>");

  boost::shared_ptr< List<Uint> > unsigned_list (allocate_component< List<Uint> >("unsigned_list"));
  BOOST_CHECK_EQUAL(unsigned_list->type_name(),"List<unsigned>");

  boost::shared_ptr< List<Real> > real_list (allocate_component< List<Real> >("real_list"));
  BOOST_CHECK_EQUAL(real_list->type_name(),"List<real>");

  boost::shared_ptr< List<std::string> > string_list (allocate_component< List<std::string> >("string_list"));
  BOOST_CHECK_EQUAL(string_list->type_name(),"List<string>");

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
  boost::shared_ptr< Table<Uint> > table (allocate_component< Table<Uint> >("table"));
  // initialize with number of columns
  Uint nbCols = 3;
  table->set_row_size(nbCols);
  // create a buffer to interact with the table
  Table<Uint>::Buffer buffer = table->create_buffer();

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
  List<Uint>& list = *root.create_component< List<Uint> >("list");
  // create a buffer to interact with the list with buffersize 3 (if no argument, use default buffersize)
  List<Uint>::Buffer buffer = list.create_buffer(3);

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
  CFinfo << list << CFendl;
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

  CFinfo << list << CFendl;


}

BOOST_AUTO_TEST_CASE ( DynTable_test )
{
  DynTable<Uint>& table = *root.create_component< DynTable<Uint> >("table");
  DynTable<Uint>::Buffer buffer = table.create_buffer();

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

  BOOST_CHECK(true);
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
  buffer.flush();

  // now the table should only have 2 row, as the 3 disabled rows should be removed

  BOOST_CHECK_EQUAL(table.size(),(Uint) 2);

  BOOST_CHECK_EQUAL(table[0][0],(Uint) 3);
  BOOST_CHECK_EQUAL(table.row_size(0), (Uint) 2);

  BOOST_CHECK_EQUAL(table[1][0],(Uint) 5);
  BOOST_CHECK_EQUAL(table.row_size(1), (Uint) 7);

}


BOOST_AUTO_TEST_CASE ( DynTable_test_hard )
{

//	0:  0
//  1:  1
//  2:  0 1
//  3:  1 4 5
//  4:  0 3
//  5:  ~
//  6:  ~
//  7:  4294967295
//  8:  ~
  DynTable<Uint>& table = *root.create_component< DynTable<Uint> >("table");
  DynTable<Uint>::Buffer buffer = table.create_buffer();

  std::vector<Uint> row;

  row = list_of(0);
  buffer.add_row(row);

  row = list_of(1);
  buffer.add_row(row);

  row = list_of(0)(1);
  buffer.add_row(row);

  row = list_of(1)(4)(5);
  buffer.add_row(row);

  row = list_of(0)(3);
  buffer.add_row(row);

  row.resize(0);
  buffer.add_row(row);

  row.resize(0);
  buffer.add_row(row);

  row.resize(0);
  buffer.add_row(row);

  row.resize(0);
  buffer.add_row(row);

  BOOST_CHECK(true);

  buffer.flush();

  buffer.rm_row(7);

  buffer.flush();


}


BOOST_AUTO_TEST_CASE ( Mesh_test )
{
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));
  Mesh& mesh = *root->create_component<Mesh>("mesh");
  Region& region = mesh.topology().create_region("region");
  Dictionary& nodes = mesh.geometry_fields();
  mesh.initialize_nodes(2,DIM_3D);
  BOOST_CHECK_EQUAL(mesh.geometry_fields().coordinates().row_size() , (Uint) DIM_3D);
}

BOOST_AUTO_TEST_CASE( List_Uint_Test )
{
  // CFinfo << "testing Table<Uint> \n" << CFflush;
  Logger::instance().getStream(DEBUG).set_log_level(SILENT);
  // Create mesh component
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  boost::shared_ptr<Mesh> mesh = allocate_component<Mesh>  ( "mesh" ) ;

  root->add_component( mesh );

  // Create one region inside mesh
  Region& region = mesh->topology().create_region("region");

  // Create connectivity table inside the region
  List<Uint>& connTable = *region.create_component<List<Uint> >("connTable");

  // check constructor
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 0);

  // check initalization
  List<Uint>::Buffer tableBuffer = connTable.create_buffer(10);

  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 0);

  // check for adding rows to table

  tableBuffer.add_row(0);
  tableBuffer.flush();
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 1);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 1);


  for (Uint i=0; i<10; ++i)
    tableBuffer.add_row(1);
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 1);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 1);

  tableBuffer.add_row(1);
  tableBuffer.flush();
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 12);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 12);

  // check if accessor / mutator works
  BOOST_CHECK_EQUAL(connTable[0], (Uint) 0);
  BOOST_CHECK_EQUAL(connTable[1], (Uint) 1);
  BOOST_CHECK_EQUAL(connTable[2], (Uint) 1);

  // check if a row can be accessed
  List<Uint>::value_type rowRef = connTable[6];

  tableBuffer.rm_row(0);
  tableBuffer.add_row(2);
  tableBuffer.add_row(3);
  tableBuffer.add_row(4);
  tableBuffer.add_row(5);
  tableBuffer.add_row(6);
  tableBuffer.rm_row(2);
  tableBuffer.rm_row(13);
  tableBuffer.flush();

  CFinfo << connTable << CFendl;

  tableBuffer.rm_row(2);
  tableBuffer.rm_row(5);
  tableBuffer.rm_row(7);
  tableBuffer.rm_row(3);
  tableBuffer.add_row(10);
  tableBuffer.add_row(10);

  tableBuffer.flush();
  CFinfo << connTable << CFendl;
}


BOOST_AUTO_TEST_CASE( List_Uint_rm_Test )
{
  // CFinfo << "testing Table<Uint> \n" << CFflush;
  Logger::instance().getStream(DEBUG).set_log_level(SILENT);
  // Create mesh component
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  boost::shared_ptr<Mesh> mesh = allocate_component<Mesh>  ( "mesh" ) ;

  root->add_component( mesh );

  // Create one region inside mesh
  Region& region = mesh->topology().create_region("region");

  // Create connectivity table inside the region
  List<Uint>& list = *region.create_component<List<Uint> >("connTable");


  // check initalization
  List<Uint>::Buffer buffer = list.create_buffer();

  // add rows to table

  buffer.add_row(0);
  buffer.add_row(1);
  buffer.add_row(2);
  buffer.add_row(3);
  buffer.add_row(6);
  buffer.add_row(7);
  buffer.add_row(8);

  buffer.flush();
  BOOST_CHECK_EQUAL( list[0] , 0u);
  BOOST_CHECK_EQUAL( list[1] , 1u);
  BOOST_CHECK_EQUAL( list[2] , 2u);
  BOOST_CHECK_EQUAL( list[3] , 3u);
  BOOST_CHECK_EQUAL( list[4] , 6u);
  BOOST_CHECK_EQUAL( list[5] , 7u);
  BOOST_CHECK_EQUAL( list[6] , 8u);

  buffer.rm_row(0);
  buffer.rm_row(3);
  buffer.rm_row(4);

  buffer.flush();
  BOOST_CHECK_EQUAL( list[0] , 7u);
  BOOST_CHECK_EQUAL( list[1] , 1u);
  BOOST_CHECK_EQUAL( list[2] , 2u);
  BOOST_CHECK_EQUAL( list[3] , 8u);
}

BOOST_AUTO_TEST_CASE( List_bool_Test )
{
  // CFinfo << "testing Table<Uint> \n" << CFflush;
  Logger::instance().getStream(DEBUG).set_log_level(SILENT);
  // Create mesh component
  boost::shared_ptr<Component> root = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));

  boost::shared_ptr<Mesh> mesh = allocate_component<Mesh>  ( "mesh" ) ;

  root->add_component( mesh );

  // Create one region inside mesh
  Region& region = mesh->topology().create_region("region");

  // Create connectivity table inside the region
  List<bool>& connTable = *region.create_component<List<bool> >("connTable");

  // check constructor
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 0);

  // check initalization
  List<bool>::Buffer tableBuffer = connTable.create_buffer(10);

  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 0);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 0);

  // check for adding rows to table

  tableBuffer.add_row(false);
  tableBuffer.flush();
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 1);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 1);

  for (Uint i=0; i<10; ++i)
    tableBuffer.add_row(true);
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 1);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 1);
  BOOST_CHECK_EQUAL(tableBuffer.get_row(10),true);

  tableBuffer.add_row(true);
  tableBuffer.flush();
  BOOST_CHECK_EQUAL(connTable.size(),(Uint) 12);
  BOOST_CHECK_EQUAL(connTable.array().num_elements(),(Uint) 12);

  // check if accessor / mutator works
  BOOST_CHECK_EQUAL(connTable[0], false);
  BOOST_CHECK_EQUAL(connTable[1], true);
  BOOST_CHECK_EQUAL(connTable[2], true);

  // check if a row can be accessed
  List<bool>::value_type rowRef = connTable[6];

  tableBuffer.rm_row(0);
  tableBuffer.flush();
  BOOST_CHECK_EQUAL(connTable[0], true);

}

//////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

