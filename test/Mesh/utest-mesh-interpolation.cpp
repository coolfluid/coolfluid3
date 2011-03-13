// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh interpolation"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "Common/OptionArray.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CLink.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CInterpolator.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CSpace.hpp"

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
  CInterpolator::Ptr interpolator = create_component_abstract_type<CInterpolator>("CF.Mesh.CLinearInterpolator","interpolator");
  BOOST_CHECK_EQUAL(interpolator->name(),"interpolator");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Interpolation )
{
  BOOST_CHECK( true );

  // create meshreader
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");

  BOOST_CHECK( true );

  boost::filesystem::path fp_source ("hextet.neu");
  CMesh::Ptr source = meshreader->create_mesh_from(fp_source);
  Core::instance().root()->add_component(source);
  BOOST_CHECK( true );

  boost::filesystem::path fp_target ("quadtriag.neu");
  CMesh::Ptr target = meshreader->create_mesh_from(fp_target);
  Core::instance().root()->add_component(target);


  BOOST_CHECK( true );

  //  boost::filesystem::path fp_target ("grid_c.cgns");
//	CMeshReader::Ptr cgns_meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.CGNS.CReader","cgns_meshreader");
//  CMesh::Ptr target = cgns_meshreader->create_mesh_from(fp_target);


  // Create and configure interpolator.
  CInterpolator::Ptr interpolator = create_component_abstract_type<CInterpolator>("CF.Mesh.CLinearInterpolator","interpolator");
  interpolator->configure_property("ApproximateNbElementsPerCell", (Uint) 1 );
  // Following configuration option has priority over the the previous one.
  std::vector<Uint> divisions = boost::assign::list_of(3)(2)(2);
  //interpolator->configure_property("Divisions", divisions );

  // Create the honeycomb
  interpolator->construct_internal_storage(*source);
  
	BOOST_CHECK(true);
	
	std::string nvars = "rho_n[1] , V_n[3] , p_n[1]";
	std::string nvars_2;
	std::string evars;
	std::string evars_2;
	
  nvars_2 = "rho_n_2[1] , V_n_2[3] , p_n_2[1]";
	evars =   "rho_e[1] , V_e[3] , p_e[1]";
  evars_2 = "rho_e_2[1] , V_e_2[3] , p_e_2[1]";
	
  // Create empty fields
  CField2& s_nodebased = source->create_field2( "nodebased", "PointBased" , "rho_n[1] , V_n[3] , p_n[1]"    );
	CField2& s_elembased = source->create_field2( "elementbased", "ElementBased" , "rho_e[1], V_e[3] , p_e[1]" );

  CField2& t_nodebased   = target->create_field2( "nodebased",   "PointBased" , "rho_n[1] , V_n[3] , p_n[1]"  );
  CField2& t_nodebased_2 = target->create_field2( "nodebased_2", "PointBased" , "rho_n_2[1] , V_n_2[3] , p_n_2[1]" );
  CField2& t_elembased   = target->create_field2( "elementbased", "ElementBased" , "rho_e[1], V_e[3] , p_e[1]" );
	
//	target->create_field2( "nodebased_2",    nvars_2, CField2::Basis::POINT_BASED    );
//	target->create_field2( "elementbased",   evars,   CField2::Basis::ELEMENT_BASED );
//	target->create_field2( "elementbased_2", evars_2, CField2::Basis::ELEMENT_BASED );
  
	BOOST_CHECK(true);
  
  for ( Uint idx=0; idx!=s_nodebased.size(); ++idx)
  {      
    CTable<Real>::ConstRow coords = s_nodebased.coords(idx);
    
    CTable<Real>::Row data = s_nodebased[idx];
    
    data[0]=coords[XX]+2.*coords[YY]+2.*coords[ZZ];
		data[1]=coords[XX];
		data[2]=coords[YY];
    data[3]=7.0;
		data[4]=coords[XX];

  }
  
  CFieldView s_elembased_view("s_elembased_view");
  s_elembased_view.set_field(s_elembased);
  RealMatrix coordinates;
  boost_foreach( CElements& elements, find_components_recursively<CElements>(s_elembased.topology()) )
  {
    if (s_elembased_view.set_elements(elements.as_ptr<CEntities>()))
    {
      s_elembased_view.allocate_coordinates(coordinates);
      RealVector coords(coordinates.rows());
      
      for (Uint elem_idx = 0; elem_idx<elements.size(); ++elem_idx)
      {
        s_elembased_view.put_coordinates( coordinates, elem_idx );
        s_elembased_view.space().shape_function().compute_centroid( coordinates , coords );
        
        s_elembased_view[elem_idx][0]=coords[XX]+2.*coords[YY]+2.*coords[ZZ];
    		s_elembased_view[elem_idx][1]=coords[XX];
    		s_elembased_view[elem_idx][2]=coords[YY];
        s_elembased_view[elem_idx][3]=7.0;
    		s_elembased_view[elem_idx][4]=coords[XX];
      }

    }
    
  }
  
  
  BOOST_CHECK(true);

  // Interpolate the source field data to the target field. Note it can be in same or different meshes
  interpolator->interpolate_field_from_to(s_nodebased,s_elembased);
  interpolator->interpolate_field_from_to(s_nodebased,t_nodebased);
  interpolator->interpolate_field_from_to(s_elembased,t_nodebased_2);
  interpolator->interpolate_field_from_to(s_elembased,t_elembased);
  
  // interpolator->interpolate_field_from_to(source->field("nodebased"),target->field("elementbased"));
  // interpolator->interpolate_field_from_to(source->field("nodebased"),source->field("elementbased"));
  // interpolator->interpolate_field_from_to(source->field("elementbased"),target->field("elementbased_2"));
  // interpolator->interpolate_field_from_to(source->field("elementbased"),target->field("nodebased_2"));
  
	BOOST_CHECK(true);

  // Write the fields to file.
  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  boost::filesystem::path fp_source_out("source.msh");
  boost::filesystem::path fp_interpolated("interpolated.msh");

  BOOST_CHECK(true);

  std::vector<CField2::Ptr> s_fields;
  boost_foreach(CField2& field, find_components_recursively<CField2>(*source))
    s_fields.push_back(field.as_ptr<CField2>());

  meshwriter->set_fields(s_fields);
	meshwriter->write_from_to(source,fp_source_out);
	BOOST_CHECK(true);


  std::vector<CField2::Ptr> t_fields;
  boost_foreach(CField2& field, find_components_recursively<CField2>(*target))
    t_fields.push_back(field.as_ptr<CField2>());

  meshwriter->set_fields(t_fields);
	meshwriter->write_from_to(target,fp_interpolated);
	BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

