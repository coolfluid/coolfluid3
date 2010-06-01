#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::LagrangeSF"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Mesh/Integrators/Gauss.hpp"

#include "Tools/Difference/Difference.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Common;

//////////////////////////////////////////////////////////////////////////////

struct Nodes_Fixture
{
  /// common setup for each test case
  Nodes_Fixture() : mesh2d(new CMesh  ( "mesh2d" ))
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

    // Read the a .neu mesh as 2D mixed mesh
    boost::shared_ptr<CMeshReader> meshreader = CMeshReader::create_concrete("Neu","meshreader");

    // the file to read from
    boost::filesystem::path fp_in ("quadtriag.neu");

    // Read the mesh
    meshreader->read_from_to(fp_in,mesh2d);
  }

  /// common tear-down for each test case
  ~Nodes_Fixture()
  {
  }
  /// common values accessed by all tests goes here
  boost::shared_ptr<CMesh> mesh2d;

  CRegion& getFirstRegion()
  {
    BOOST_FOREACH(CRegion& region, iterate_recursive_by_type<CRegion>(*mesh2d))
    {
      if(region.getNbElements())
        return (region);
    }
    throw ShouldNotBeHere(FromHere(), "");
  }

  CArray::Ptr coordinatesPtr() {
    return mesh2d->get_component<CArray>("coordinates");
  }

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Nodes, Nodes_Fixture )

//////////////////////////////////////////////////////////////////////////////

/// Test two methods for writing to nodes
BOOST_AUTO_TEST_CASE( writeNodes )
{
  CRegion& firstRegion = getFirstRegion();
  CArray::Ptr coords = coordinatesPtr();
  CArray::Row firstCoord = firstRegion.get_row(0, 0, coords);
  firstCoord[XX] = 1.;
  firstCoord[YY] = 1.;
  CArray::Row modCoord = firstRegion.get_row(0, 0, coords);
  BOOST_CHECK_EQUAL(modCoord[XX], 1.);
  BOOST_CHECK_EQUAL(modCoord[YY], 1.);
  CRegion::ElementNodeVector nodes = firstRegion.getNodes(0, *coords);
  nodes[0][XX] = 2.;
  nodes[0][YY] = 2.;
  BOOST_CHECK_EQUAL(modCoord[XX], 2.);
  BOOST_CHECK_EQUAL(modCoord[YY], 2.);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

