#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/ConfigObject.hpp"
#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Math/RealVector.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct MeshData_Fixture
{
  /// common setup for each test case
  MeshData_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~MeshData_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshData_TestSuite, MeshData_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( FieldTest )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  
  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");
  // the mesh to store in
  CMesh& mesh = *meshreader->create_mesh_from(fp_in);
  
  mesh.create_field("Volume",1,get_component_typed<CRegion>(mesh));
  BOOST_CHECK_EQUAL(mesh.get_child("Volume")->full_path().string(),"mesh/Volume");
  
  CFinfo << mesh.tree() << CFendl;
  
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
  
}

////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

