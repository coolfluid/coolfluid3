#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::LagrangeSF"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/Integrators/IntegrationFunctorBase.hpp"

#include "Tools/Difference/Difference.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Common;
//////////////////////////////////////////////////////////////////////////////

struct Integration_Fixture
{
  /// common setup for each test case
  Integration_Fixture() : mesh2Dneu(new CMesh  ( "mesh2Dneu" )), meshUnitSquare(new CMesh  ( "meshUnitSquare" ))
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

    // Read the a .neu mesh as 2D mixed mesh
    boost::shared_ptr<CMeshReader> meshreader = CMeshReader::create_concrete("Neu","meshreader");
    boost::filesystem::path fp_in ("quadtriag.neu");
    meshreader->read_from_to(fp_in,mesh2Dneu);

    // Create a square, 2D, quad-only mesh, sized 1x1
    const Real x_len = 1.;
    const Real y_len = 1.;
    const Uint dim = 2;
    CArray& coordinates = *meshUnitSquare->create_array("coordinates");
    coordinates.initialize(dim);
    CArray::Buffer coordinatesBuffer = coordinates.create_buffer();
    const Uint x_segments = 200;
    const Uint y_segments = 200;
    const Real x_step = x_len / static_cast<Real>(x_segments);
    const Real y_step = y_len / static_cast<Real>(y_segments);
    std::vector<Real> coords(dim);
    for(Uint j = 0; j <= y_segments; ++j)
    {
      coords[YY] = static_cast<Real>(j) * y_step;
      for(Uint i = 0; i <= x_segments; ++i)
      {
        coords[XX] = static_cast<Real>(i) * x_step;
        coordinatesBuffer.add_row(coords);
      }
    }
    CRegion& region = *meshUnitSquare->create_region("region");
    CTable& connTable = *region.create_connectivityTable("connectivity_table");
    connTable.initialize(4); // 4 nodes per element
    region.create_elementType("element_type")->set_elementType("P1-Quad2D");
    CTable::Buffer connBuffer = connTable.create_buffer();
    std::vector<Uint> nodes(4);
    for(Uint j = 0; j < y_segments; ++j)
    {
      for(Uint i = 0; i < x_segments; ++i)
      {
        nodes[0] = j * (x_segments+1) + i;
        nodes[1] = nodes[0] + 1;
        nodes[3] = (j+1) * (x_segments+1) + i;
        nodes[2] = nodes[3] + 1;
        connBuffer.add_row(nodes);
      }
    }
    coordinatesBuffer.flush();
    connBuffer.flush();
  }

  /// common tear-down for each test case
  ~Integration_Fixture()
  {
  }
  /// common values accessed by all tests goes here
  boost::shared_ptr<CMesh> mesh2Dneu;
  boost::shared_ptr<CMesh> meshUnitSquare;


  /// Returns the determinant of the jacobian, i.e. integrating this over the entire mesh
  /// should yield the volume of the mesh
  struct DetJacobianFunctor : public IntegrationFunctorBase
  {
    DetJacobianFunctor(const CArray& coordinates) : IntegrationFunctorBase(coordinates) {}

    template<typename GeoShapeF, typename SolShapeF>
    CF::Real valTimesDetJacobian(const CF::RealVector& mappedCoords)
    {
      return GeoShapeF::computeJacobianDeterminant(mappedCoords, m_nodes);
    }
  };

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Integration, Integration_Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( computeVolume2D )
{
  // Compute the volume of the mesh, by integrating a functor
  DetJacobianFunctor ftor2Dneu(*(mesh2Dneu->get_component<CArray>("coordinates")));
  Real volume2Dneu = 0.0;
  gaussIntegrate(*mesh2Dneu, ftor2Dneu, volume2Dneu);
  BOOST_CHECK_CLOSE(volume2Dneu, 24., 1e-12);

  DetJacobianFunctor ftor2Dbig(*(meshUnitSquare->get_component<CArray>("coordinates")));
  Real volume2Dbig = 0.0;
  gaussIntegrate(*meshUnitSquare, ftor2Dbig, volume2Dbig);
  BOOST_CHECK_CLOSE(volume2Dbig, 1., 1e-8);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

