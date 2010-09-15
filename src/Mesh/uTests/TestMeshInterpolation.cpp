// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

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
                     
  CInterpolator::Ptr interpolator = create_component_abstract_type<CInterpolator>("Honeycomb","interpolator");
  
  
  std::string text = (
                      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                      "<cfxml version=\"1.0\">"
                      "  <signal>"
                      "    <valuemap>"
                      ""
                      "      <value  key=\"ApproximateNbElementsPerCell\"> <unsigned> 1 </unsigned> </value>"
                      ""
//                      "      <array key=\"Divisions\" type=\"unsigned\" size=\"3\" >"
//                      "        <e> 3 </e>"
//                      "        <e> 2 </e>"
//                      "        <e> 2 </e>"
//                      "      </array>"
                      ""
                      "    </valuemap>"
                      "  </signal>"
                      "</cfxml>"
                      );
  
  boost::shared_ptr<XmlDoc> xml = XmlOps::parse(text);
  XmlNode& doc   = *XmlOps::goto_doc_node(*xml.get());
  XmlNode& frame = *XmlOps::first_frame_node( doc );
  interpolator->configure( frame );
  
  
  //interpolator->construct_internal_storage(source,target);
  
  
  source->create_field("rho",1,CField::NODE_BASED);
  target->create_field("rho",1,CField::NODE_BASED);
  
  
  BOOST_FOREACH(CFieldElements& field_elements, recursive_range_typed<CFieldElements>(*target))
  {    
    CArray& node_data = field_elements.data();
    CArray& coordinates = field_elements.coordinates();
    
    for (Uint i=0; i<coordinates.size(); ++i)
    {
      node_data[i][0]=sin(3*2*3.141592*coordinates[i][XX]);
    }
  }
  
  
  //interpolator->interpolate_field_from_to(source->field("rho"),target->field("rho"));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

