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
  
  /// These are handy functions that should maybe be implemented somewhere easily accessible.
  
  /// create a Real vector with 2 coordinates
  RealVector create_coord(const Real& x, const Real& y) {
    RealVector coordVec(2);
    coordVec[XX]=x;
    coordVec[YY]=y;
    return coordVec;
  }
  
  /// create a Uint vector with 4 node ID's
  std::vector<Uint> create_quad(const Uint& A, const Uint& B, const Uint& C, const Uint& D) {
    Uint quad[] = {A,B,C,D};
    std::vector<Uint> quadVec;
    quadVec.assign(quad,quad+4);
    return quadVec;
  }
  
  /// create a Uint vector with 3 node ID's
  std::vector<Uint> create_triag(const Uint& A, const Uint& B, const Uint& C) {
    Uint triag[] = {A,B,C};
    std::vector<Uint> triagVec;
    triagVec.assign(triag,triag+3);
    return triagVec;
  }
  
  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshConstruction_TestSuite, MeshConstruction_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( MeshConstruction )
{
  
  
  // Create root and mesh component
  boost::shared_ptr<CRoot> root = CRoot::create ( "root" );

  boost::shared_ptr<Component> mesh ( new CMesh  ( "mesh" ) );

  root->add_component( mesh );
  
  // create a mesh pointer
  boost::shared_ptr<CMesh> p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  // create regions
  p_mesh->create_region("superRegion");
  p_mesh->get_component<CRegion>("superRegion")->create_region("quads");
  p_mesh->get_component<CRegion>("superRegion")->create_region("triags");
  
  // create a pointer to the quads and triag region
  boost::shared_ptr<CRegion> superRegion =
    p_mesh->get_component<CRegion>("superRegion"); 
  boost::shared_ptr<CRegion> quadRegion =
    superRegion->get_component<CRegion>("quads"); 
  boost::shared_ptr<CRegion> triagRegion =
    superRegion->get_component<CRegion>("triags"); 

  // create connectivity table and element type in the quads and triags region
  quadRegion->create_connectivityTable("table");
  quadRegion->create_elementType("type");
  triagRegion->create_connectivityTable("table");
  triagRegion->create_elementType("type");
  
  // set the element types
  quadRegion->get_component<CElements>("type")->set_elementType("Quad2D");
  triagRegion->get_component<CElements>("type")->set_elementType("Triag2D");

  // create a coordinates array in the mesh component
  p_mesh->create_array("coordinates");
  
  // create pointers to the coordinates array and connectivity table
  boost::shared_ptr<CArray> coordinates = p_mesh->get_component<CArray>("coordinates");
  boost::shared_ptr<CTable> qTable = quadRegion->get_component<CTable>("table");
  boost::shared_ptr<CTable> tTable = triagRegion->get_component<CTable>("table");

  // initialize the coordinates array and connectivity tables
  const Uint dim=2;
  coordinates->initialize(dim);
  qTable->initialize(4);
  tTable->initialize(3);
  CTable::Buffer qTableBuffer = qTable->create_buffer();
  CTable::Buffer tTableBuffer = tTable->create_buffer();
  CArray::Buffer coordinatesBuffer = coordinates->create_buffer();
  
  //  Mesh of quads and triangles with node and element numbering:
  //
  //    5----4----6    
  //    |    |\ 3 |
  //    |    | \  |
  //    | 1  |  \ |
  //    |    | 2 \|
  //    3----2----7
  //    |    |\ 1 |
  //    |    | \  |
  //    | 0  |  \ |
  //    |    | 0 \|
  //    0----1----8   
  
  // fill coordinates in the buffer
  coordinatesBuffer.add_row(create_coord( 0.0 , 0.0 ));  // 0
  coordinatesBuffer.add_row(create_coord( 1.0 , 0.0 ));  // 1
  coordinatesBuffer.add_row(create_coord( 1.0 , 1.0 ));  // 2
  coordinatesBuffer.add_row(create_coord( 0.0 , 1.0 ));  // 3
  coordinatesBuffer.add_row(create_coord( 1.0 , 2.0 ));  // 4
  coordinatesBuffer.add_row(create_coord( 0.0 , 2.0 ));  // 5
  coordinatesBuffer.add_row(create_coord( 2.0 , 2.0 ));  // 6
  coordinatesBuffer.add_row(create_coord( 2.0 , 1.0 ));  // 7
  coordinatesBuffer.add_row(create_coord( 2.0 , 0.0 ));  // 8

  
  // fill connectivity in the buffer
  qTableBuffer.add_row(create_quad( 0 , 1 , 2 , 3 ));
  qTableBuffer.add_row(create_quad( 3 , 2 , 4 , 5 ));

  tTableBuffer.add_row(create_triag( 1 , 8 , 2 ));
  tTableBuffer.add_row(create_triag( 8 , 7 , 2 ));
  tTableBuffer.add_row(create_triag( 2 , 7 , 4 ));
  tTableBuffer.add_row(create_triag( 7 , 6 , 4 ));

  // flush buffers into the table. 
  // This causes the table and array to be resized and filled.
  coordinatesBuffer.flush();
  qTableBuffer.flush();
  tTableBuffer.flush();
  
  // check if coordinates match (3 ways)
  Uint elem=1;
  Uint node=2;
  boost::array<Real,2> coord;
  quadRegion->set_row(coord,elem,node,coordinates);
  BOOST_CHECK_EQUAL(coord[0],1.0);
  BOOST_CHECK_EQUAL(coord[1],2.0);
  
  std::vector<Real> stlcoord(2);
  triagRegion->set_row(stlcoord,elem,node,coordinates);
  BOOST_CHECK_EQUAL(stlcoord[0],1.0);
  BOOST_CHECK_EQUAL(stlcoord[1],1.0);
  
  CArray::Row coordRef = triagRegion->get_row(elem,node,coordinates);
  BOOST_CHECK_EQUAL(coordRef[0],1.0);
  BOOST_CHECK_EQUAL(coordRef[1],1.0);

//  // calculate all volumes of a region
//  BOOST_FOREACH(boost::shared_ptr<CRegion> region,superRegion->get_subregions())
//  {
//    boost::shared_ptr<CElements>  elementType = region->get_component<CElements>("type");
//    boost::shared_ptr<CTable>     connTable   = region->get_component<CTable>("table");
//    // CFinfo << "type = " << elementType->getShapeName() << "\n" << CFendl;
//    const Uint nbRows = connTable->get_table().size();
//    std::vector<Real> volumes(nbRows);
//    
//    // the loop
//    for (Uint iElem=0; iElem<nbRows; ++iElem) {
//      std::vector<CArray::Row > elementCoordinates;
//      for (Uint iNode=0; iNode<elementType->getNbNodes(); iNode++) {
//        elementCoordinates.push_back(region->get_row(iElem,iNode,coordinates));
//      }
//      volumes[iElem]=elementType->computeVolume(elementCoordinates);
//      // CFinfo << "\t volume["<<iElem<<"] =" << volumes[iElem] << "\n" << CFendl;
//
//      // check
//      if(elementType->getShapeName()=="Quad")
//        BOOST_CHECK_EQUAL(volumes[iElem],1.0);
//      if(elementType->getShapeName()=="Triag")
//        BOOST_CHECK_EQUAL(volumes[iElem],0.5);
//    }
    //
  //}
    
  
//  BOOST_FOREACH(CArray::Row node , elementCoordinates)
//  {
//    CFinfo << "node = ";
//    for (Uint j=0; j<node.size(); j++) {
//      CFinfo << node[j] << " ";
//    }
//    CFinfo << "\n" << CFendl;
//  }
      
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

