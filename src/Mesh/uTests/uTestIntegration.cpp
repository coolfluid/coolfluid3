#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for integration methods"

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
#include "Mesh/ElementNodes.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/Integrators/IntegrationFunctorBase.hpp"

#include "Tools/Testing/ProfiledTestFixture.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Common;
using namespace CF::Tools;
using namespace CF::Tools::Testing;

namespace detail { // helper functions

/// Create a rectangular, 2D, quad-only mesh. Uses a buffer for insertion
void create_rectangle_buffered(CMesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments) {
  const Uint dim = 2;
  CArray& coordinates = *mesh.create_array("coordinates");
  coordinates.initialize(dim);
  CArray::Buffer coordinatesBuffer = coordinates.create_buffer( (x_segments+1)*(y_segments+1) );
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
  CRegion& region = *mesh.create_region("region");
  CTable::Buffer connBuffer = region.create_elements("Quad2DLagrangeP1").connectivity_table().create_buffer( x_segments*y_segments );
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

/// Create a rectangular, 2D, quad-only mesh. No buffer for creation
void create_rectangle(CMesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments) {
  CFinfo << "Creating 2D rectangular grid" << CFendl;
  const Uint dim = 2;
  CArray& coordinates = *mesh.create_array("coordinates");
  coordinates.initialize(dim);
  CArray::Array& coordArray = coordinates.array();
  coordArray.resize(boost::extents[(x_segments+1)*(y_segments+1)][dim]);
  const Real x_step = x_len / static_cast<Real>(x_segments);
  const Real y_step = y_len / static_cast<Real>(y_segments);
  Real y;
  for(Uint j = 0; j <= y_segments; ++j)
  {
    y = static_cast<Real>(j) * y_step;
    for(Uint i = 0; i <= x_segments; ++i)
    {
      CArray::Row row = coordArray[j*(x_segments+1)+i];
      row[XX] = static_cast<Real>(i) * x_step;
      row[YY] = y;
    }
  }
  CRegion& region = *mesh.create_region("region");
  CTable::ConnectivityTable& connArray = region.create_elements("Quad2DLagrangeP1").connectivity_table().table();
  connArray.resize(boost::extents[(x_segments)*(y_segments)][4]);
  for(Uint j = 0; j < y_segments; ++j)
  {
    for(Uint i = 0; i < x_segments; ++i)
    {
      CTable::Row nodes = connArray[j*(x_segments)+i];
      nodes[0] = j * (x_segments+1) + i;
      nodes[1] = nodes[0] + 1;
      nodes[3] = (j+1) * (x_segments+1) + i;
      nodes[2] = nodes[3] + 1;
    }
  }
}

} // namespace detail

//////////////////////////////////////////////////////////////////////////////

/// Use a global fixture, so mesh creation happens outside of the profiling
struct GlobalFixture {

  GlobalFixture() {
    if(!grid2D) {
      grid2D.reset(new CMesh("grid2D"));
      detail::create_rectangle(*grid2D, 1., 1., 500, 500);
    }
  }

  static CMesh::Ptr grid2D;
};

CMesh::Ptr GlobalFixture::grid2D = CMesh::Ptr();

//////////////////////////////////////////////////////////////////////////////

/// Profile and time tests using this fixture
struct IntegrationFixture :
  public ProfiledTestFixture, // NOTE: Inheritance order matters, this way the timing is profiled,
  public TimedTestFixture     //       but the profiling is not timed. Important since especially profile processing takes time.
{
  IntegrationFixture() : grid2D(*GlobalFixture::grid2D) {}

  /// Returns the determinant of the jacobian, i.e. integrating this over the entire mesh
  /// should yield the volume of the mesh
  struct DetJacobianFunctor : public IntegrationFunctorBase
  {
    DetJacobianFunctor(const CArray& coordinates) : IntegrationFunctorBase(coordinates) {}

    template<typename GeoShapeF, typename SolShapeF>
    CF::Real valTimesDetJacobian(const CF::RealVector& mappedCoords)
    {
      return GeoShapeF::jacobian_determinant(mappedCoords, m_nodes);
    }
  };

  /// Use a vector for node storage
  struct DetJacobianFunctorNodesVector {
    /// Sets up a functor for the given mesh
    DetJacobianFunctorNodesVector(const CArray& coordinates) : m_coordinates(coordinates) {}
    /// Sets up the functor to use the specified region
    void setRegion(const CElements& region) {
      m_region = &region;
      m_nodes.resize(region.element_type().nb_nodes(), RealVector(region.element_type().dimension()));
    }
    /// Sets up the functor to use the specified element (relative to the currently set region)
    void setElement(const Uint element) {
      const CTable::ConnectivityTable& ctbl = m_region->connectivity_table().table();
      Uint i = 0;
      BOOST_FOREACH(const Uint idx, ctbl[element])
      {
        m_nodes[i++] = RealVector(m_coordinates[idx]);
      }
    }

    template<typename GeoShapeF, typename SolShapeF>
    CF::Real valTimesDetJacobian(const CF::RealVector& mappedCoords)
    {
      return GeoShapeF::jacobian_determinant(mappedCoords, m_nodes);
    }

  protected:
    const CElements* m_region;
    const CArray& m_coordinates;
    ElementNodeVector m_nodes;
  };

  /// Access to a pre-generated 2D grid
  CMesh& grid2D;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( GlobalFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( Integration )

//////////////////////////////////////////////////////////////////////////////

/// Profile mesh creation with a buffer
BOOST_FIXTURE_TEST_CASE( CreateMeshBuffered, IntegrationFixture ) // timed and profiled
{
  boost::shared_ptr<CMesh> mesh(new CMesh("mesh"));
  detail::create_rectangle_buffered(*mesh, 1., 1., 1000, 1000);
}

/// Profile direct mesh creation
BOOST_FIXTURE_TEST_CASE( CreateMeshRaw, IntegrationFixture ) // timed and profiled
{
  boost::shared_ptr<CMesh> mesh(new CMesh("mesh"));
  detail::create_rectangle(*mesh, 1., 1., 1000, 1000);
}

/// Test integration over a 2D gambit .neu mesh
BOOST_AUTO_TEST_CASE( ComputeVolume2DNeu ) // not timed
{
  // Read the a .neu mesh as 2D mixed mesh
  boost::shared_ptr<CMeshReader> meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  boost::filesystem::path fp_in ("quadtriag.neu");
  boost::shared_ptr<CMesh> mesh2Dneu(new CMesh("mesh2Dneu"));
  meshreader->read_from_to(fp_in,mesh2Dneu);
  IntegrationFixture::DetJacobianFunctor ftor2Dneu(get_named_component_typed<CArray>(*mesh2Dneu, "coordinates"));
  Real volume2Dneu = 0.0;
  gaussIntegrate(*mesh2Dneu, ftor2Dneu, volume2Dneu);
  BOOST_CHECK_CLOSE(volume2Dneu, 24., 1e-12);
}

/// Use the computeVolume virtual function to calculate the volume
BOOST_FIXTURE_TEST_CASE( ComputeVolume2DUnitSquare, IntegrationFixture ) // timed and profiled
{
  CArray& coords = get_named_component_typed<CArray>(grid2D, "coordinates");
  Real volume = 0.0;
  BOOST_FOREACH(CElements& region, recursive_range_typed<CElements>(grid2D))
  {
    const CTable::ConnectivityTable& ctbl = region.connectivity_table().table();
    const Uint element_count = ctbl.size();
    const ElementType& element_type = region.element_type();
    for(Uint element = 0; element != element_count; ++element)
    {
      ElementType::NodesT nodes;
      BOOST_FOREACH(const Uint idx, ctbl[element])
      {
        nodes.push_back(RealVector(coords[idx]));
      }
      volume += element_type.computeVolume(nodes);
    }
  }
  BOOST_CHECK_CLOSE(volume, 1., 1e-8);
}

/// Directly compute the volume using the node coordinates
BOOST_FIXTURE_TEST_CASE( ComputeVolume2DUnitSquareDirect, IntegrationFixture ) // timed and profiled
{
  CArray& coords = get_named_component_typed<CArray>(grid2D, "coordinates");
  Real volume = 0.0;
  BOOST_FOREACH(CElements& region, recursive_range_typed<CElements>(grid2D))
  {
    const CTable::ConnectivityTable& ctbl = region.connectivity_table().table();
    const Uint element_count = ctbl.size();
    const ElementType& element_type = region.element_type();
    for(Uint element = 0; element != element_count; ++element)
    {
      ElementType::NodesT nodes;
      BOOST_FOREACH(const Uint idx, ctbl[element])
      {
        nodes.push_back(RealVector(coords[idx]));
      }
      volume += (nodes[2][XX] - nodes[0][XX]) * (nodes[3][YY] - nodes[1][YY]) -
          (nodes[2][YY] - nodes[0][YY]) * (nodes[3][XX] - nodes[1][XX]);
    }
  }
  volume *= 0.5;
  BOOST_CHECK_CLOSE(volume, 1., 1e-8);
}

/// Compute the volume by integrating a functor, using a view of the nodes
BOOST_FIXTURE_TEST_CASE( IntegrateVolume2DUnitSquareNodeView, IntegrationFixture ) // timed and profiled
{
  DetJacobianFunctor ftor2Dbig(get_named_component_typed<CArray>(grid2D, "coordinates"));
  Real volume2Dbig = 0.0;
  gaussIntegrate(grid2D, ftor2Dbig, volume2Dbig);
  BOOST_CHECK_CLOSE(volume2Dbig, 1., 1e-8);
}

/// Compute the volume by integrating a functor, using a copy of the nodes
BOOST_FIXTURE_TEST_CASE( IntegrateVolume2DUnitSquareNodeVector, IntegrationFixture ) // timed and profiled
{
  DetJacobianFunctorNodesVector ftor2Dbig(get_named_component_typed<CArray>(grid2D, "coordinates"));
  Real volume2Dbig = 0.0;
  gaussIntegrate(grid2D, ftor2Dbig, volume2Dbig);
  BOOST_CHECK_CLOSE(volume2Dbig, 1., 1e-8);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

