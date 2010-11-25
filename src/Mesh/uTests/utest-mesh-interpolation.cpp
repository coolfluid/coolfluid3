// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh interpolation"

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/OptionArray.hpp"
#include "Common/XmlHelpers.hpp"

#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CLink.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CInterpolator.hpp"

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct MeshInterpolation_Fixture
{
  /// common setup for each test case
  MeshInterpolation_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~MeshInterpolation_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshInterpolation_TestSuite, MeshInterpolation_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors)
{
  CInterpolator::Ptr interpolator = create_component_abstract_type<CInterpolator>("Honeycomb","interpolator");
  BOOST_CHECK_EQUAL(interpolator->name(),"interpolator");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Interpolation )
{
  // build meshes
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  boost::filesystem::path fp_source ("hextet.neu");
  CMesh::Ptr source = meshreader->create_mesh_from(fp_source);
  boost::filesystem::path fp_target ("quadtriag.neu");
	CMesh::Ptr target = meshreader->create_mesh_from(fp_target);
                     
//  boost::filesystem::path fp_target ("grid_c.cgns");	
//	CMeshReader::Ptr cgns_meshreader = create_component_abstract_type<CMeshReader>("CGNS","cgns_meshreader");
//  CMesh::Ptr target = cgns_meshreader->create_mesh_from(fp_target);


	// Create and configure interpolator.
  CInterpolator::Ptr interpolator = create_component_abstract_type<CInterpolator>("Honeycomb","interpolator");
	interpolator->configure_property("ApproximateNbElementsPerCell", (Uint) 1 );
	// Following configuration option has priority over the the previous one.
	std::vector<Uint> divisions = boost::assign::list_of(3)(2)(2);
	//interpolator->configure_property("Divisions", divisions ); 
	
  // Create the honeycomb
  interpolator->construct_internal_storage(source);
  
	BOOST_CHECK(true);
	
	std::vector<std::string> nvars;
	std::vector<std::string> nvars_2;
	std::vector<std::string> evars;
	std::vector<std::string> evars_2;
	
	nvars +=   "rho_n[1]"   , "V_n[3]"   , "p_n[1]";
  nvars_2 += "rho_n_2[1]" , "V_n_2[3]" , "p_n_2[1]";
	evars +=   "rho_e[1]"   , "V_e[3]"   , "p_e[1]";
  evars_2 += "rho_e_2[1]" , "V_e_2[3]" , "p_e_2[1]";
	
  // Create empty fields
  source->create_field( "nodebased",      nvars,   CField::NODE_BASED    );
	source->create_field( "elementbased",   evars,   CField::ELEMENT_BASED );

  target->create_field( "nodebased",      nvars,   CField::NODE_BASED    );
	target->create_field( "nodebased_2",    nvars_2, CField::NODE_BASED    );
	target->create_field( "elementbased",   evars,   CField::ELEMENT_BASED );
	target->create_field( "elementbased_2", evars_2, CField::ELEMENT_BASED );
  
	BOOST_CHECK(true);
	
  // Set the field data of the source field
  BOOST_FOREACH(CArray& node_data, recursive_filtered_range_typed<CArray>(*source,IsComponentTag("node_data")))
  {    
		CFinfo << node_data.full_path().string() << CFendl;
		CArray& coordinates = *node_data.get_child_type<CLink>("coordinates")->get_type<CArray>();

    for (Uint i=0; i<coordinates.size(); ++i)
		{
			node_data[i][0]=coordinates[i][XX]+2.*coordinates[i][YY]+2.*coordinates[i][ZZ];
			node_data[i][1]=coordinates[i][XX];
			node_data[i][2]=coordinates[i][YY];
			node_data[i][3]=7.0;
			node_data[i][4]=coordinates[i][XX];
		}
  }

  // Interpolate the source field data to the target field. Note it can be in same or different meshes
  interpolator->interpolate_field_from_to(source->field("nodebased"),target->field("nodebased"));
	interpolator->interpolate_field_from_to(source->field("nodebased"),target->field("elementbased"));
	interpolator->interpolate_field_from_to(source->field("nodebased"),source->field("elementbased"));
	interpolator->interpolate_field_from_to(source->field("elementbased"),target->field("elementbased_2"));
	interpolator->interpolate_field_from_to(source->field("elementbased"),target->field("nodebased_2"));
	
	BOOST_CHECK(true);
	
	// Write the fields to file.
	CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
	boost::filesystem::path fp_source_out("source.msh");
	boost::filesystem::path fp_interpolated("interpolated.msh");
	BOOST_CHECK(true);

	meshwriter->write_from_to(source,fp_source_out);
	BOOST_CHECK(true);

	meshwriter->write_from_to(target,fp_interpolated);
	BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

