// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::CField"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/CLink.hpp"

#include "Math/RealVector.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CArray.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

struct FieldTests_Fixture
{
  /// common setup for each test case
  FieldTests_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

    CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");

    // the file to read from
    boost::filesystem::path fp_in ("quadtriag.neu");
    // the mesh to store in
    m_mesh = meshreader->create_mesh_from(fp_in);

  }

  /// common tear-down for each test case
  ~FieldTests_Fixture()
  {
  }

	/// possibly common functions used on the tests below

	/// create a Real vector with 2 coordinates
  RealVector create_coord(const Real& x, const Real& y) {
    RealVector coordVec(2);
    coordVec[XX]=x;
    coordVec[YY]=y;
    return coordVec;
  }
	
	std::vector<Uint> create_quad_p2(const Uint A, const Uint B, const Uint C, const Uint D,
																	 const Uint E, const Uint F, const Uint G, const Uint H, const Uint I) 
	{
    Uint quad[] = {A,B,C,D,E,F,G,H,I};
    std::vector<Uint> quadVec;
    quadVec.assign(quad,quad+9);
    return quadVec;
  }
	
  std::vector<Uint> create_triag_p2(const Uint A, const Uint B, const Uint C,
																		const Uint D, const Uint E, const Uint F) 
	{
    Uint triag[] = {A,B,C,D,E,F};
    std::vector<Uint> triagVec;
    triagVec.assign(triag,triag+6);
    return triagVec;
  }

  /// common values accessed by all tests goes here

  CMesh::Ptr m_mesh;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( FieldTests_TestSuite, FieldTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( FieldTest )
{
  CMesh& mesh = *m_mesh;

  mesh.create_field("Volume",1,CField::ELEMENT_BASED);
	std::vector<std::string> solution_vars = list_of("rho[1]")("V[3]")("p[1]");
  mesh.create_field("Solution",solution_vars,CField::NODE_BASED);

	CFinfo << mesh.tree() << CFendl;
	
  // Check if the fields have been created inside the mesh
  BOOST_CHECK_EQUAL(mesh.field("Volume").full_path().string(),"mesh/Volume");
  BOOST_CHECK_EQUAL(mesh.field("Solution").full_path().string(),"mesh/Solution");

  // Check if support is filled in correctly
  BOOST_CHECK_EQUAL(mesh.field("Volume").support().name(), "quadtriag");
  BOOST_CHECK_EQUAL(mesh.field("Volume").support().recursive_filtered_elements_count(IsElementsVolume()), (Uint) 16);
  BOOST_CHECK_EQUAL(mesh.field("Volume").subfield("gas").support().recursive_elements_count(), (Uint) 6);

  // Check if connectivity_table is properly linked to the support ones
  BOOST_CHECK_EQUAL(mesh.field("Volume").subfield("gas").elements("elements_Quad2DLagrangeP1").connectivity_table().size(), (Uint) 2);
  BOOST_CHECK_EQUAL(&mesh.field("Volume").subfield("gas").elements("elements_Quad2DLagrangeP1").connectivity_table(),
                    &mesh.domain().subregion("gas").elements("elements_Quad2DLagrangeP1").connectivity_table());

  // test the CRegion::get_field function, to return the matching field
  BOOST_CHECK_EQUAL(mesh.domain().get_field("Volume").full_path().string(),"mesh/Volume");
  BOOST_CHECK_EQUAL(mesh.domain().subregion("gas").get_field("Volume").full_path().string(),"mesh/Volume/gas");

  BOOST_CHECK_EQUAL(mesh.look_component("quadtriag/gas")->full_path().string(),"mesh/quadtriag/gas");
  BOOST_CHECK_EQUAL(mesh.look_component("quadtriag/gas/../liquid")->full_path().string(),"mesh/quadtriag/liquid");
  BOOST_CHECK_EQUAL(mesh.look_component_type<CRegion>("quadtriag/gas/../liquid")->get_field("Volume").full_path().string(),"mesh/Volume/liquid");


  // Check if element based data is correctly created
  BOOST_CHECK_EQUAL(mesh.look_component_type<CFieldElements>("Volume/gas/elements_Quad2DLagrangeP1")->data().size(), (Uint) 2);
  BOOST_CHECK_EQUAL(mesh.look_component_type<CFieldElements>("Volume/gas/elements_Quad2DLagrangeP1")->data().row_size(), (Uint) 1);

  // Check if node based data is correctly created
  BOOST_CHECK_EQUAL(mesh.look_component_type<CFieldElements>("Solution/gas/elements_Quad2DLagrangeP1")->data().row_size(), (Uint) 5);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( P2_2D_FieldTest )
{
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
    // 1.5-  16 17  14 22  23
    //    |  |       |   \  |
  	//    |  |  (1)  |(2) \ |
  	//    |  |       |     \|
    //  1 -  3--11---2--21--7
    //    |  |       |\     |
    //    |  |       | \ (1)|
    // 0.5-  12 13  10 19  20
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
	
	
	BOOST_CHECK(true);
	
	// create the field
	std::vector<std::string> solution_vars = list_of("rho[1]")("V[3]")("p[1]");
  p_mesh->create_field("Solution",solution_vars,CField::NODE_BASED);

	// Set the field data of the source field
  BOOST_FOREACH(CArray& node_data, recursive_filtered_range_typed<CArray>(*p_mesh,IsComponentTag("node_data")))
  {    
		CFinfo << node_data.full_path().string() << CFendl;
		CArray& coordinates = *node_data.get_child_type<CLink>("coordinates")->get_type<CArray>();
		CFLogVar(node_data.size());
		CFLogVar(coordinates.size());
		CFLogVar(node_data[0].size());
    for (Uint i=0; i<coordinates.size(); ++i)
		{
			CFLogVar(i);
			node_data[i][0]=coordinates[i][XX]+2.*coordinates[i][YY];
			node_data[i][1]=coordinates[i][XX];
			node_data[i][2]=coordinates[i][YY];
			node_data[i][3]=7.0;
			node_data[i][4]=coordinates[i][XX];
		}
  }
	BOOST_CHECK(true);
	
	CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
	boost::filesystem::path fp_out("p2-field.msh");
	meshwriter->write_from_to(p_mesh,fp_out);
	
}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

