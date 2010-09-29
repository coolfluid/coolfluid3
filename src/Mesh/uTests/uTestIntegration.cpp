// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

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

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Common;
using namespace CF::Tools;
using namespace CF::Tools::Testing;
using namespace CF::Tools::MeshGeneration;

namespace detail { // helper functions

/// Create a rectangular, 2D, quad-only mesh. Uses a buffer for insertion
void create_rectangle_buffered(CMesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments) {
  const Uint dim = 2;
  CArray& coordinates = *mesh.create_component_type<CArray>("coordinates");
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
  CRegion& region = mesh.create_region("region");
  CTable::Buffer connBuffer = region.create_elements("Quad2DLagrangeP1",coordinates).connectivity_table().create_buffer( x_segments*y_segments );
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

} // namespace detail

//////////////////////////////////////////////////////////////////////////////

/// Define the global fixture type
typedef MeshSourceGlobalFixture<2000> MeshSource;

/// Profile and time tests using this fixture
struct IntegrationFixture :
  public ProfiledTestFixture, // NOTE: Inheritance order matters, this way the timing is profiled,
  public TimedTestFixture     //       but the profiling is not timed. Important since especially profile processing takes time.
{
  IntegrationFixture() : grid2D(MeshSource::grid2()) {}

  /// Returns the determinant of the jacobian, i.e. integrating this over the entire mesh
  /// should yield the volume of the mesh
  struct DetJacobianFunctor : public IntegrationFunctorBase
  {
    DetJacobianFunctor() : IntegrationFunctorBase() {}

    template<typename GeoShapeF, typename SolShapeF>
    CF::Real valTimesDetJacobian(const CF::RealVector& mappedCoords)
    {
      return GeoShapeF::jacobian_determinant(mappedCoords, m_nodes);
    }
  };

  /// Use a vector for node storage
  struct DetJacobianFunctorNodesVector {
    /// Sets up a functor for the given mesh
    DetJacobianFunctorNodesVector() {}
    /// Sets up the functor to use the specified region
    void setRegion(const CElements& region) {
      m_region = &region;
      m_coordinates = &region.coordinates();
      m_nodes.resize(region.element_type().nb_nodes(), RealVector(region.element_type().dimension()));
    }
    /// Sets up the functor to use the specified element (relative to the currently set region)
    void setElement(const Uint element) {
      const CTable::ArrayT& ctbl = m_region->connectivity_table().array();
      Uint i = 0;
      BOOST_FOREACH(const Uint idx, ctbl[element])
      {
        m_nodes[i++] = RealVector((*m_coordinates)[idx]);
      }
    }

    template<typename GeoShapeF, typename SolShapeF>
    CF::Real valTimesDetJacobian(const CF::RealVector& mappedCoords)
    {
      return GeoShapeF::jacobian_determinant(mappedCoords, m_nodes);
    }

  protected:
    const CElements* m_region;
    const CArray* m_coordinates;
    ElementNodeVector m_nodes;
  };

  /// Access to a pre-generated 2D grid
  const CMesh& grid2D;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( MeshSource )

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
  create_rectangle(*mesh, 1., 1., 1000, 1000);
}

/// Test integration over a 2D gambit .neu mesh
BOOST_AUTO_TEST_CASE( ComputeVolume2DNeu ) // not timed
{
  // Read the a .neu mesh as 2D mixed mesh
  boost::shared_ptr<CMeshReader> meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  boost::filesystem::path fp_in ("quadtriag.neu");
  boost::shared_ptr<CMesh> mesh2Dneu(new CMesh("mesh2Dneu"));
  meshreader->read_from_to(fp_in,mesh2Dneu);
  IntegrationFixture::DetJacobianFunctor ftor2Dneu;
  Real volume2Dneu = 0.0;
  gaussIntegrate(*mesh2Dneu, ftor2Dneu, volume2Dneu);
  BOOST_CHECK_CLOSE(volume2Dneu, 24., 1e-12);
}

/// Use the computeVolume virtual function to calculate the volume
BOOST_FIXTURE_TEST_CASE( ComputeVolume2DUnitSquare, IntegrationFixture ) // timed and profiled
{
  const CArray& coords = get_named_component_typed<CArray>(grid2D, "coordinates");
  Real volume = 0.0;
  BOOST_FOREACH(const CElements& region, recursive_range_typed<CElements>(grid2D))
  {
    const CTable::ArrayT& ctbl = region.connectivity_table().array();
    const Uint element_count = ctbl.size();
    const ElementType& element_type = region.element_type();
    ElementType::NodesT nodes(element_type.nb_nodes(), element_type.dimension());
    for(Uint element = 0; element != element_count; ++element)
    {
      nodes.fill(coords, ctbl[element]);
      volume += element_type.computeVolume(nodes);
    }
  }
  BOOST_CHECK_CLOSE(volume, 1., 1e-8);
}

/// Directly compute the volume using the node coordinates
BOOST_FIXTURE_TEST_CASE( ComputeVolume2DUnitSquareDirectVector, IntegrationFixture ) // timed and profiled
{
  const CArray& coords = get_named_component_typed<CArray>(grid2D, "coordinates");
  Real volume = 0.0;
  BOOST_FOREACH(const CElements& region, recursive_range_typed<CElements>(grid2D))
  {
    const CTable::ArrayT& ctbl = region.connectivity_table().array();
    const Uint element_count = ctbl.size();
    ElementNodeVector nodes;
    for(Uint element = 0; element != element_count; ++element)
    {
      fill_node_list(nodes, coords, ctbl[element]);
      volume += (nodes[2][XX] - nodes[0][XX]) * (nodes[3][YY] - nodes[1][YY]) -
          (nodes[2][YY] - nodes[0][YY]) * (nodes[3][XX] - nodes[1][XX]);
    }
  }
  volume *= 0.5;
  BOOST_CHECK_CLOSE(volume, 1., 1e-8);
}

/// Directly compute the volume using the node coordinates. ElementNodes version
BOOST_FIXTURE_TEST_CASE( ComputeVolume2DUnitSquareDirectMemCopy, IntegrationFixture ) // timed and profiled
{
  const CArray& coords = get_named_component_typed<CArray>(grid2D, "coordinates");
  Real volume = 0.0;
  BOOST_FOREACH(const CElements& region, recursive_range_typed<CElements>(grid2D))
  {
    const CTable::ArrayT& ctbl = region.connectivity_table().array();
    const Uint element_count = ctbl.size();
    ElementNodes nodes(4,2);
    for(Uint element = 0; element != element_count; ++element)
    {
      nodes.fill(coords, ctbl[element]);
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
  DetJacobianFunctor ftor2Dbig;
  Real volume2Dbig = 0.0;
  gaussIntegrate(grid2D, ftor2Dbig, volume2Dbig);
  BOOST_CHECK_CLOSE(volume2Dbig, 1., 1e-8);
}

/// Compute the volume by integrating a functor, using a copy of the nodes
BOOST_FIXTURE_TEST_CASE( IntegrateVolume2DUnitSquareNodeVector, IntegrationFixture ) // timed and profiled
{
  DetJacobianFunctorNodesVector ftor2Dbig;
  Real volume2Dbig = 0.0;
  gaussIntegrate(grid2D, ftor2Dbig, volume2Dbig);
  BOOST_CHECK_CLOSE(volume2Dbig, 1., 1e-8);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

