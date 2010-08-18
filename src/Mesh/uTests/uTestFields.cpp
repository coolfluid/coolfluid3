#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::CField"

#include <boost/test/unit_test.hpp>

#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/OptionT.hpp"
#include "Common/Log.hpp"

#include "Math/RealVector.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

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
  

  /// common values accessed by all tests goes here

  CMesh::Ptr m_mesh;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( FieldTests_TestSuite, FieldTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( FieldTest )
{
  CMesh& mesh = *m_mesh;
  
  mesh.create_field("Volume",1,get_component_typed<CRegion>(mesh));
  mesh.create_field("Solution",5,get_component_typed<CRegion>(mesh));
  BOOST_CHECK_EQUAL(mesh.get_child("Volume")->full_path().string(),"mesh/Volume");
  BOOST_CHECK_EQUAL(mesh.get_child("Solution")->full_path().string(),"mesh/Solution");
    
  // Check if support is filled in correctly
  BOOST_CHECK_EQUAL(mesh.get_child_type<CField>("Volume")->support().name(), "regions");
  BOOST_CHECK_EQUAL(mesh.get_child_type<CField>("Volume")->support().recursive_filtered_elements_count(IsElementsVolume()), (Uint) 16);
  BOOST_CHECK_EQUAL(mesh.get_child("Volume")->get_child_type<CField>("gas")->support().recursive_elements_count(), (Uint) 6);

  // Check if data is correctly created
  BOOST_CHECK_EQUAL(mesh.get_child_type<CField>("Volume")->recursive_elements_count(), (Uint) 28);
  BOOST_CHECK_EQUAL(mesh.get_child("Volume")->get_child_type<CArray>("data")->size() , (Uint) 0);
  
  // Check if connectivity_table is properly linked to the support ones
  BOOST_CHECK_EQUAL(mesh.get_child("Volume")->get_child("gas")->get_child_type<CElements>("elements_Quad2DLagrangeP1")->connectivity_table().size(), (Uint) 2);
  BOOST_CHECK_EQUAL(&mesh.get_child("Volume")->get_child("gas")->get_child_type<CElements>("elements_Quad2DLagrangeP1")->connectivity_table(),
                    &mesh.get_child("regions")->get_child("gas")->get_child_type<CElements>("elements_Quad2DLagrangeP1")->connectivity_table());
  
  //  CFinfo << mesh.tree() << CFendl;
  
  // test the CRegion::get_field function, to return the matching field
  BOOST_CHECK_EQUAL(mesh.get_child_type<CRegion>("regions")->get_field("Volume").full_path().string(),"mesh/Volume");
  BOOST_CHECK_EQUAL(mesh.get_child("regions")->get_child_type<CRegion>("gas")->get_field("Volume").full_path().string(),"mesh/Volume/gas");
    
  BOOST_CHECK_EQUAL(mesh.look_component("regions/gas")->full_path().string(),"mesh/regions/gas");
  BOOST_CHECK_EQUAL(mesh.look_component("regions/gas/../liquid")->full_path().string(),"mesh/regions/liquid");
  BOOST_CHECK_EQUAL(mesh.look_component_type<CRegion>("regions/gas/../liquid")->get_field("Volume").full_path().string(),"mesh/Volume/liquid");
  
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

