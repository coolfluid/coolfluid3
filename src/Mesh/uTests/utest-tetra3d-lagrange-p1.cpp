// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Tetra3DLagrangeP1 shape function"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Tetra3DLagrangeP1.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::SF;
using namespace CF::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Tetra3DLagrangeP1 SFT;

struct Tetra3DLagrangeP1Fixture
{
  typedef SFT::NodeMatrixT NodesT;
  /// common setup for each test case
  Tetra3DLagrangeP1Fixture() :
    mapped_coords(0.1, 0.8, 0.45),
    nodes
    (
      (NodesT() <<
        0.830434, 0.885201, 0.188108, 
        0.89653, 0.899961, 0.297475,
        0.888273, 0.821744, 0.211428, 
        0.950439, 0.904872, 0.20736).finished()
    ),
    volume(1.451803461048456186e-4)
  {
  }

  /// common tear-down for each test case
  ~Tetra3DLagrangeP1Fixture()
  {
  }
  /// common values accessed by all tests goes here

  const SFT::MappedCoordsT mapped_coords;
  const NodesT nodes;
  const Real volume;

  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      return Tetra3DLagrangeP1::jacobian_determinant(mapped_coords, m_nodes);
    }
    SFT::CoordsT mapped_coords;
  private:
    const NodesT& m_nodes;
  };
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Tetra3DLagrangeP1Suite, Tetra3DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const SFT::ShapeFunctionsT reference_result(-0.35, 0.1, 0.8, 0.45);
  SFT::ShapeFunctionsT result;
  Tetra3DLagrangeP1::shape_function(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedCoordinates )
{
  const SFT::MappedCoordsT test_coords(0.92, 0.87, 0.21);
  const SFT::CoordsT reference_result(1.779178467272182762e-02, 4.106555656123735409e-01, 5.386286149811901902e-01);
  SFT::MappedCoordsT result;
  Tetra3DLagrangeP1::mapped_coordinates(test_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  ConstFunctor ftor(nodes);
  Real result = 0.0;
  gauss_integrate<1, GeoShape::TETRA>(ftor, ftor.mapped_coords, result);
  BOOST_CHECK_CLOSE(result, volume, 0.0000000001);
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  SFT::MappedGradientT expected;
  expected(XX, 0) = -1.;
  expected(YY, 0) = -1.;
  expected(ZZ, 0) = -1.;
  expected(XX, 1) = 1.;
  expected(YY, 1) = 0.;
  expected(ZZ, 1) = 0.;
  expected(XX, 2) = 0.;
  expected(YY, 2) = 1.;
  expected(ZZ, 2) = 0.;
  expected(XX, 3) = 0.;
  expected(YY, 3) = 0.;
  expected(ZZ, 3) = 1.;
  SFT::MappedGradientT result;
  Tetra3DLagrangeP1::mapped_gradient(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianDeterminant )
{
  // Shapefunction determinant should be a sixth of the volume for tetrahedra
  const Real result = Tetra3DLagrangeP1::jacobian_determinant(mapped_coords, nodes)/6.;
  BOOST_CHECK_CLOSE(result, volume, 0.00001);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  SFT::JacobianT expected;
  expected(0,0) = 6.609600000000004361e-02;
  expected(0,1) = 1.475999999999999535e-02;
  expected(0,2) = 1.093669999999999920e-01;
  expected(1,0) = 5.783899999999997377e-02;
  expected(1,1) = -6.345699999999998564e-02;
  expected(1,2) = 2.332000000000000739e-02;
  expected(2,0) = 1.200050000000000283e-01;
  expected(2,1) = 1.967099999999999405e-02;
  expected(2,2) = 1.925199999999999134e-02;
  SFT::JacobianT result;
  Tetra3DLagrangeP1::jacobian(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianAdjoint )
{
  SFT::JacobianT expected;
  expected(0,0) = -1.680401883999999273e-03;
  expected(0,1) = 1.867198736999999475e-03;
  expected(0,2) = 7.284304918999997755e-03;
  expected(1,0) = 1.685000172000002431e-03;
  expected(1,1) = -1.185210664300000161e-02;
  expected(1,2) = 4.784319192999994877e-03;
  expected(2,0) = 8.752908253999998334e-03;
  expected(2,1) = 4.710993839999994925e-04;
  expected(2,2) = -5.047957512000001042e-03;
  SFT::JacobianT result;
  Tetra3DLagrangeP1::jacobian_adjoint(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Is_coord_in_element )
{
  const SFT::CoordsT centroid = nodes.colwise().sum() / SFT::nb_nodes;
  
  BOOST_CHECK_EQUAL(Tetra3DLagrangeP1::in_element(centroid,nodes),true);
  
  BOOST_CHECK_EQUAL(Tetra3DLagrangeP1::in_element(nodes.row(0),nodes),true);
  BOOST_CHECK_EQUAL(Tetra3DLagrangeP1::in_element(nodes.row(1),nodes),true);
  BOOST_CHECK_EQUAL(Tetra3DLagrangeP1::in_element(nodes.row(2),nodes),true);
  BOOST_CHECK_EQUAL(Tetra3DLagrangeP1::in_element(nodes.row(3),nodes),true);

  BOOST_CHECK_EQUAL(Tetra3DLagrangeP1::in_element(2.0 * centroid,nodes),false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

