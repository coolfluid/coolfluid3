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
  boost::filesystem::path fp_in ("quadtriag.neu");
  CMesh::Ptr source = meshreader->create_mesh_from(fp_in);
  CMesh::Ptr target (new CMesh ("target"));
                     
  CInterpolator::Ptr interpolator = create_component_abstract_type<CInterpolator>("Honeycomb","interpolator");
  interpolator->interpolate_from_to(source,target);

  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

