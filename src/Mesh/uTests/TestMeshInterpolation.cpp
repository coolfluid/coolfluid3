// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/assign/list_of.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/PropertyT.hpp"
#include "Common/PropertyArray.hpp"
#include "Common/PropertyComponent.hpp"
#include "Common/XmlHelpers.hpp"

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Math/RealVector.hpp"

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
  interpolator->construct_internal_storage(source,target);
  
  // Create empty fields
  source->create_field("rho",1,CField::NODE_BASED);
  target->create_field("rho",1,CField::NODE_BASED);
  
  // Set the field data of the source field
  BOOST_FOREACH(CFieldElements& field_elements, recursive_range_typed<CFieldElements>(*source))
  {    
    CArray& node_data = field_elements.data();
    CArray& coordinates = field_elements.coordinates();
    
    for (Uint i=0; i<coordinates.size(); ++i)
      node_data[i][0]=2*coordinates[i][XX];
  }
  
  // Interpolate the source field data to the target field. Note it can be in same or different meshes
  interpolator->interpolate_field_from_to(source->field("rho"),target->field("rho"));
	
	// Write the fields to file.
	CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
	boost::filesystem::path fp_source_out("source.msh");
	boost::filesystem::path fp_interpolated("interpolated.msh");
	meshwriter->write_from_to(source,fp_source_out);
	meshwriter->write_from_to(target,fp_interpolated);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

