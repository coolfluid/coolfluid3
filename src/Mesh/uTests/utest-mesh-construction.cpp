// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh construction"


#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/ElementType.hpp"

#include "Mesh/CMeshWriter.hpp"

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

	std::vector<Uint> create_quad_p2(const Uint A, const Uint B, const Uint C, const Uint D,
																	 const Uint E, const Uint F, const Uint G, const Uint H, const Uint I) 
	{
    Uint quad[] = {A,B,C,D,E,F,G,H,I};
    std::vector<Uint> quadVec;
    quadVec.assign(quad,quad+9);
    return quadVec;
  }
	
  /// create a Uint vector with 3 node ID's
  std::vector<Uint> create_triag_p2(const Uint A, const Uint B, const Uint C,
																		const Uint D, const Uint E, const Uint F) 
	{
    Uint triag[] = {A,B,C,D,E,F};
    std::vector<Uint> triagVec;
    triagVec.assign(triag,triag+6);
    return triagVec;
  }
	
  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshConstruction_TestSuite, MeshConstruction_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( P1_2D_MeshConstruction )
{
  AssertionManager::instance().AssertionThrows = true;

  const Uint dim=2;

  // Create root and mesh component
  CRoot::Ptr root = CRoot::create ( "root" );

  Component::Ptr mesh ( new CMesh  ( "mesh" ) );

  root->add_component( mesh );

  // create a mesh pointer
  CMesh::Ptr p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);

  // create regions
  CRegion& superRegion = p_mesh->create_region("superRegion");
  CArray& coordinates = superRegion.create_coordinates(dim);
  CElements& quadRegion = superRegion.create_elements("Quad2DLagrangeP1",coordinates);
  CElements& triagRegion = superRegion.create_elements("Triag2DLagrangeP1",coordinates);

  CTable::Buffer qTableBuffer = quadRegion.connectivity_table().create_buffer();
  CTable::Buffer tTableBuffer = triagRegion.connectivity_table().create_buffer();
  CArray::Buffer coordinatesBuffer = coordinates.create_buffer();

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

  // check if coordinates match
  Uint elem=1;
  Uint node=2;

  CTable::ConstRow nodesRef = triagRegion.connectivity_table()[elem];
  CArray::Row coordRef = triagRegion.coordinates()[nodesRef[node]];
  BOOST_CHECK_EQUAL(coordRef[0],1.0);
  BOOST_CHECK_EQUAL(coordRef[1],1.0);

  // calculate all volumes of a region
  BOOST_FOREACH( CElements& region, recursive_range_typed<CElements>(superRegion))
  {
    const ElementType& elementType = region.element_type();
    const Uint nbRows = region.connectivity_table().size();
    std::vector<Real> volumes(nbRows);
    const CArray& region_coordinates = region.coordinates();
    const CTable& region_connTable = region.connectivity_table();
    // the loop
    ElementType::NodesT elementCoordinates(elementType.nb_nodes(), elementType.dimension());
    for (Uint iElem=0; iElem<nbRows; ++iElem)
    {
      fill(elementCoordinates, region_coordinates, region_connTable[iElem]);

      volumes[iElem]=elementType.computeVolume(elementCoordinates);

      // check
      if(elementType.shape() == GeoShape::QUAD)
        BOOST_CHECK_EQUAL(volumes[iElem],1.0);
      if(elementType.shape() == GeoShape::TRIAG)
        BOOST_CHECK_EQUAL(volumes[iElem],0.5);
    }
  }

//  BOOST_FOREACH(CArray::Row node , elem_coord)
//  {
//    CFinfo << "node = ";
//    for (Uint j=0; j<node.size(); j++) {
//      CFinfo << node[j] << " ";
//    }
//    CFinfo << "\n" << CFflush;
//  }
	
	CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
	boost::filesystem::path fp_out("p1-mesh.msh");
	meshwriter->write_from_to(p_mesh,fp_out);
	

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( P2_2D_MeshConstruction )
{
  AssertionManager::instance().AssertionThrows = true;
	
  const Uint dim=2;
	
  // Create root and mesh component
  CRoot::Ptr root = CRoot::create ( "root" );
	
  Component::Ptr mesh ( new CMesh  ( "mesh" ) );
	
  root->add_component( mesh );
	
  // create a mesh pointer
  CMesh::Ptr p_mesh = boost::dynamic_pointer_cast<CMesh>(mesh);
	
  // create regions
  CRegion& superRegion = p_mesh->create_region("superRegion");
  CArray& coordinates = superRegion.create_coordinates(dim);
  CElements& quadRegion = superRegion.create_elements("Quad2DLagrangeP2",coordinates);
  CElements& triagRegion = superRegion.create_elements("Triag2DLagrangeP2",coordinates);
	
  CTable::Buffer qTableBuffer = quadRegion.connectivity_table().create_buffer();
  CTable::Buffer tTableBuffer = triagRegion.connectivity_table().create_buffer();
  CArray::Buffer coordinatesBuffer = coordinates.create_buffer();
	
  //  Mesh of quads and triangles with node numbering and element numbering in brackets:
  //
	//  Y ^
	//    |
  //  2 -  5--15---4--24--6
  //    |  |       |\     |
  //    |  |       | \ (3)|
  // 1.5-  16 17  14  22 23
  //    |  |       |   \  |
	//    |  |  (1)  |(2) \ |
	//    |  |       |     \|
  //  1 -  3--11---2--21--7
  //    |  |       |\     |
  //    |  |       | \ (1)|
  // 0.5-  12 13  10  19 20
  //    |  |       |   \  |
	//    |  |  (0)  | (0)\ |
	//    |  |       |     \|
  //  0 -  0---9---1--18--8
	//    |
	//    o--|---|---|--|--|------> X
	//       0  0.5  1 1.5  2
	
  // fill coordinates in the buffer of the P1 mesh
  coordinatesBuffer.add_row(create_coord( 0.0 , 0.0 ));  // 0
  coordinatesBuffer.add_row(create_coord( 1.0 , 0.0 ));  // 1
  coordinatesBuffer.add_row(create_coord( 1.0 , 1.0 ));  // 2
  coordinatesBuffer.add_row(create_coord( 0.0 , 1.0 ));  // 3
  coordinatesBuffer.add_row(create_coord( 1.0 , 2.0 ));  // 4
  coordinatesBuffer.add_row(create_coord( 0.0 , 2.0 ));  // 5
  coordinatesBuffer.add_row(create_coord( 2.0 , 2.0 ));  // 6
  coordinatesBuffer.add_row(create_coord( 2.0 , 1.0 ));  // 7
  coordinatesBuffer.add_row(create_coord( 2.0 , 0.0 ));  // 8
	
	// enrich
	coordinatesBuffer.add_row(create_coord( 0.5 , 0.0 ));  // 9
	coordinatesBuffer.add_row(create_coord( 1.0 , 0.5 ));  // 10
	coordinatesBuffer.add_row(create_coord( 0.5 , 1.0 ));  // 11
	coordinatesBuffer.add_row(create_coord( 0.0 , 0.5 ));  // 12
	coordinatesBuffer.add_row(create_coord( 0.5 , 0.5 ));  // 13
	coordinatesBuffer.add_row(create_coord( 1.0 , 1.5 ));  // 14
	coordinatesBuffer.add_row(create_coord( 0.5 , 2.0 ));  // 15
	coordinatesBuffer.add_row(create_coord( 0.0 , 1.5 ));  // 16
	coordinatesBuffer.add_row(create_coord( 0.5 , 1.5 ));  // 17
	coordinatesBuffer.add_row(create_coord( 1.5 , 0.0 ));  // 18
  coordinatesBuffer.add_row(create_coord( 1.5 , 0.5 ));  // 19
	coordinatesBuffer.add_row(create_coord( 2.0 , 0.5 ));  // 20
	coordinatesBuffer.add_row(create_coord( 1.5 , 1.0 ));  // 21
	coordinatesBuffer.add_row(create_coord( 1.5 , 1.5 ));  // 22
	coordinatesBuffer.add_row(create_coord( 2.0 , 1.5 ));  // 23
	coordinatesBuffer.add_row(create_coord( 1.5 , 2.0 ));  // 24
	
	
	// enrich
	
  // fill connectivity in the buffer
  qTableBuffer.add_row(create_quad_p2( 0 , 1 , 2 , 3 , 9 , 10, 11, 12, 13 ));
  qTableBuffer.add_row(create_quad_p2( 3 , 2 , 4 , 5 , 11, 14, 15, 16, 17 ));
	
  tTableBuffer.add_row(create_triag_p2( 1 , 8 , 2 , 18 , 19 , 10 ));
  tTableBuffer.add_row(create_triag_p2( 8 , 7 , 2 , 20 , 21 , 19 ));
  tTableBuffer.add_row(create_triag_p2( 2 , 7 , 4 , 21 , 22 , 14 ));
  tTableBuffer.add_row(create_triag_p2( 7 , 6 , 4 , 23 , 24 , 22 ));
	
  // flush buffers into the table.
  // This causes the table and array to be resized and filled.
  coordinatesBuffer.flush();
  qTableBuffer.flush();
  tTableBuffer.flush();
	
  // check if coordinates match
  Uint elem=1;
  Uint node=2;
	
  CTable::ConstRow nodesRef = triagRegion.connectivity_table()[elem];
  CArray::Row coordRef = triagRegion.coordinates()[nodesRef[node]];
  BOOST_CHECK_EQUAL(coordRef[0],1.0);
  BOOST_CHECK_EQUAL(coordRef[1],1.0);
	
//  // calculate all volumes of a region
//  BOOST_FOREACH( CElements& region, recursive_range_typed<CElements>(superRegion))
//  {
//    const ElementType& elementType = region.element_type();
//    const Uint nbRows = region.connectivity_table().size();
//    std::vector<Real> volumes(nbRows);
//    const CArray& region_coordinates = region.coordinates();
//    const CTable& region_connTable = region.connectivity_table();
//    // the loop
//    ElementType::NodesT elementCoordinates(elementType.nb_nodes(), elementType.dimension());
//    for (Uint iElem=0; iElem<nbRows; ++iElem)
//    {
//      elementCoordinates.fill(region_coordinates, region_connTable[iElem]);
//			
//      volumes[iElem]=elementType.computeVolume(elementCoordinates);
//			
//      // check
//      if(elementType.shape() == GeoShape::QUAD)
//        BOOST_CHECK_EQUAL(volumes[iElem],1.0);
//      if(elementType.shape() == GeoShape::TRIAG)
//        BOOST_CHECK_EQUAL(volumes[iElem],0.5);
//    }
//  }
//	
//	//  BOOST_FOREACH(CArray::Row node , elem_coord)
//	//  {
//	//    CFinfo << "node = ";
//	//    for (Uint j=0; j<node.size(); j++) {
//	//      CFinfo << node[j] << " ";
//	//    }
//	//    CFinfo << "\n" << CFflush;
//	//  }
	
	
	CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
	boost::filesystem::path fp_out("p2-mesh.msh");
	meshwriter->write_from_to(p_mesh,fp_out);
	
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

