// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Quad2DLagrangeP1 shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/ElementNodes.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Quad2DLagrangeP1.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::SF;
using namespace CF::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

struct LagrangeSFQuad2DLagrangeP1Fixture
{
  typedef std::vector<RealVector> NodesT;
  /// common setup for each test case
  LagrangeSFQuad2DLagrangeP1Fixture() : mapped_coords(init_mapped_coords()), nodes(init_nodes())
  {
  }

  /// common tear-down for each test case
  ~LagrangeSFQuad2DLagrangeP1Fixture()
  {
  }
  /// common values accessed by all tests goes here

  const CF::RealVector mapped_coords;
  const NodesT nodes;

  struct const_functor
  {
    const_functor(const NodesT& node_list) : m_nodes(node_list) {}
    template<typename GeoShapeF, typename SolShapeF>
    CF::Real valTimesDetJacobian(const CF::RealVector& mappedCoords)
    {
      return GeoShapeF::jacobian_determinant(mappedCoords, m_nodes);
    }
  private:
    const NodesT& m_nodes;
  };

private:
  /// Workaround for boost:assign ambiguity
  CF::RealVector init_mapped_coords()
  {
    return list_of(0.1)(0.8);
  }

  /// Workaround for boost:assign ambiguity
  NodesT init_nodes()
  {
    const CF::RealVector c0 = list_of(0.5)(0.3);
    const CF::RealVector c1 = list_of(1.1)(1.2);
    const CF::RealVector c2 = list_of(1.35)(1.9);
    const CF::RealVector c3 = list_of(0.8)(2.1);
    return list_of(c0)(c1)(c2)(c3);
  }
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Quad2DLagrangeP1Suite, LagrangeSFQuad2DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  boost::multi_array<Real,2> nodes_quad2D (boost::extents[4][2]);
  nodes_quad2D[0][XX] = 0.0;     nodes_quad2D[0][YY] = 0.0;
  nodes_quad2D[1][XX] = 1.0;     nodes_quad2D[1][YY] = 0.0;
  nodes_quad2D[2][XX] = 1.0;     nodes_quad2D[2][YY] = 1.0;
  nodes_quad2D[3][XX] = 0.0;     nodes_quad2D[3][YY] = 1.0;
  BOOST_CHECK_EQUAL(Quad2DLagrangeP1::volume(nodes_quad2D), 1.0);
}

BOOST_AUTO_TEST_CASE( Element )
{
  // Create a CElements component
  CElements::Ptr comp (new CElements("comp")) ;
  CArray::Ptr coordinates (new CArray("coordinates"));
  comp->initialize("Quad2DLagrangeP1",*coordinates);
  BOOST_CHECK_EQUAL(comp->element_type().shape(), GeoShape::QUAD);
  BOOST_CHECK_EQUAL(comp->element_type().nb_faces(), (Uint) 4);

  // Check volume calculation
  CArray::ArrayT coord(boost::extents[4][2]);
  coord[0][XX]=15; coord[0][YY]=15;
  coord[1][XX]=40; coord[1][YY]=25;
  coord[2][XX]=25; coord[2][YY]=30;
  coord[3][XX]=30; coord[3][YY]=40;
  std::vector<CArray::Row> coordvec;
  coordvec.reserve(4);
  coordvec.push_back(coord[0]);
  coordvec.push_back(coord[1]);
  coordvec.push_back(coord[2]);
  coordvec.push_back(coord[3]);
  
  BOOST_CHECK_EQUAL(Quad2DLagrangeP1::volume(coordvec), 150.); 
}

BOOST_AUTO_TEST_CASE( computeShapeFunction )
{
  const CF::RealVector reference_result = list_of(0.045)(0.055)(0.495)(0.405);
  CF::RealVector result(4);
  Quad2DLagrangeP1::shape_function(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( computeMappedCoordinates )
{
  Accumulator accumulator;
  CF::RealVector test_coords = list_of(0.9375)(1.375); // center of the element
  CF::RealVector result(2);
  Quad2DLagrangeP1::mapped_coordinates(test_coords, nodes, result);
  BOOST_CHECK_LT(std::abs(result[0]), 1e-12);
  BOOST_CHECK_LT(std::abs(result[1]), 1e-12);// sqrt from the expression gives too many ULPS in difference for Accumulator
  
  test_coords = nodes[0];
  Quad2DLagrangeP1::mapped_coordinates(test_coords, nodes, result);  
  vector_test(result,point2(-1.0, -1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;
  
  test_coords = nodes[1];
  Quad2DLagrangeP1::mapped_coordinates(test_coords, nodes, result);  
  vector_test(result,point2(1.0, -1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;
  
  test_coords = nodes[2];
  Quad2DLagrangeP1::mapped_coordinates(test_coords, nodes, result);  
  vector_test(result,point2(1.0, 1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;
  
  test_coords = nodes[3];
  Quad2DLagrangeP1::mapped_coordinates(test_coords, nodes, result);  
  vector_test(result,point2(-1.0, 1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;
  
  // Try another element
  const CF::RealVector c0 = list_of(1.0)(1.0);
  const CF::RealVector c1 = list_of(2.0)(1.0);
  const CF::RealVector c2 = list_of(2.0)(2.0);
  const CF::RealVector c3 = list_of(1.0)(2.0);
  NodesT nodes_2 = list_of(c0)(c1)(c2)(c3);

  test_coords = nodes_2[0];
  Quad2DLagrangeP1::mapped_coordinates(test_coords, nodes_2, result);  
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;
  vector_test(result,point2(-1.0, -1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  
  test_coords = nodes_2[1];
  Quad2DLagrangeP1::mapped_coordinates(test_coords, nodes_2, result);  
  vector_test(result,point2(1.0, -1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;
  
  test_coords = nodes_2[2];
  Quad2DLagrangeP1::mapped_coordinates(test_coords, nodes_2, result);  
  vector_test(result,point2(1.0, 1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;
  
  test_coords = nodes_2[3];
  Quad2DLagrangeP1::mapped_coordinates(test_coords, nodes_2, result);  
  vector_test(result,point2(-1.0, 1.0),accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 600);
  CFinfo << "result[0] = " << result[0] << CFendl;
  CFinfo << "result[1] = " << result[1] << CFendl << CFendl;
}

BOOST_AUTO_TEST_CASE( computeMappedGradient )
{
  CF::RealMatrix expected(Quad2DLagrangeP1::dimensionality, Quad2DLagrangeP1::nb_nodes);
  const CF::Real ksi  = mapped_coords[0];
  const CF::Real eta = mapped_coords[1];
  expected(0,0) = 0.25 * (-1 + eta);
  expected(1,0) = 0.25 * (-1 + ksi);
  expected(0,1) = 0.25 * ( 1 - eta);
  expected(1,1) = 0.25 * (-1 - ksi);
  expected(0,2) = 0.25 * ( 1 + eta);
  expected(1,2) = 0.25 * ( 1 + ksi);
  expected(0,3) = 0.25 * (-1 - eta);
  expected(1,3) = 0.25 * ( 1 - ksi);
  CF::RealMatrix result(Quad2DLagrangeP1::dimensionality, Quad2DLagrangeP1::nb_nodes);
  Quad2DLagrangeP1::mapped_gradient(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( computeJacobianDeterminant )
{
  // Shapefunction determinant at center should be a quarter of the cell volume
  const CF::RealVector center_coords = list_of(0.)(0.);
  BOOST_CHECK_LT(boost::accumulators::max(test(4.*Quad2DLagrangeP1::jacobian_determinant(center_coords, nodes), Quad2DLagrangeP1::volume(nodes)).ulps), 1);
}

BOOST_AUTO_TEST_CASE( computeJacobian )
{
  CF::RealMatrix expected(2, 2);
  expected(0,0) = 0.2775;
  expected(0,1) = -0.045;
  expected(1,0) = 0.13625;
  expected(1,1) = 0.5975;
  CF::RealMatrix result(2, 2);
  Quad2DLagrangeP1::jacobian(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 15);
}

BOOST_AUTO_TEST_CASE( computeJacobianAdjoint )
{
  CF::RealMatrix expected(2, 2);
  expected(0,0) = 0.5975;
  expected(0,1) = 0.045;
  expected(1,0) = -0.13625;
  expected(1,1) = 0.2775;
  CF::RealMatrix result(2, 2);
  Quad2DLagrangeP1::jacobian_adjoint(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 15);
}

BOOST_AUTO_TEST_CASE( integrateConst )
{
  // Shapefunction determinant should be double the volume for triangles
  const_functor ftor(nodes);
  const Real vol = Quad2DLagrangeP1::volume(nodes);

  CF::Real result1 = 0.0;
  CF::Real result2 = 0.0;
  CF::Real result4 = 0.0;
  CF::Real result8 = 0.0;
  CF::Real result16 = 0.0;
  CF::Real result32 = 0.0;

  Gauss<Quad2DLagrangeP1>::integrateElement(ftor, result1);
  Gauss<Quad2DLagrangeP1, Quad2DLagrangeP1, 2>::integrateElement(ftor, result2);
  Gauss<Quad2DLagrangeP1, Quad2DLagrangeP1, 4>::integrateElement(ftor, result4);
  Gauss<Quad2DLagrangeP1, Quad2DLagrangeP1, 8>::integrateElement(ftor, result8);
  Gauss<Quad2DLagrangeP1, Quad2DLagrangeP1, 16>::integrateElement(ftor, result16);
  Gauss<Quad2DLagrangeP1, Quad2DLagrangeP1, 32>::integrateElement(ftor, result32);

  BOOST_CHECK_LT(boost::accumulators::max(test(result1, vol).ulps), 1);
  BOOST_CHECK_LT(boost::accumulators::max(test(result2, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result4, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result8, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result16, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result32, vol).ulps), 5);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

