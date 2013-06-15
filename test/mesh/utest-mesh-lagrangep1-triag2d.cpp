// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for ETYPE"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"

#include "mesh/ContinuousDictionary.hpp"
#include "mesh/Integrators/Gauss.hpp"
#include "mesh/LagrangeP1/Triag2D.hpp"
#include "mesh/Elements.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::Integrators;
using namespace cf3::mesh::LagrangeP1;

//////////////////////////////////////////////////////////////////////////////

typedef Triag2D ETYPE;

struct LagrangeP1Triag2DFixture
{
  typedef ETYPE::NodesT NodesT;
  /// common setup for each test case
  LagrangeP1Triag2DFixture() : mapped_coords(0.1, 0.8), nodes((NodesT() << 0.5, 0.3,
                                                                           1.1, 1.2,
                                                                           0.8, 2.1).finished())
  {
  }

  /// common tear-down for each test case
  ~LagrangeP1Triag2DFixture()
  {
  }
  /// common values accessed by all tests goes here

  const ETYPE::MappedCoordsT mapped_coords;
  const NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      return ETYPE::jacobian_determinant(mapped_coords, m_nodes);
    }
    ETYPE::MappedCoordsT mapped_coords;
  private:
    const NodesT& m_nodes;
  };
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LagrangeP1Triag2DSuite, LagrangeP1Triag2DFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  NodesT nodes_triag2D;
  nodes_triag2D <<
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0;
  BOOST_CHECK_EQUAL(ETYPE::volume(nodes_triag2D), 0.5);
}

BOOST_AUTO_TEST_CASE( Element )
{
  boost::shared_ptr<Dictionary> nodes = allocate_component<ContinuousDictionary>("nodes") ;
  // Create a Elements component
  boost::shared_ptr<Elements> comp = allocate_component<Elements>("comp");

  comp->initialize("cf3.mesh.LagrangeP1.Triag2D",*nodes);
  BOOST_CHECK_EQUAL(comp->element_type().shape(), GeoShape::TRIAG);
  BOOST_CHECK_EQUAL(comp->element_type().nb_nodes(), (Uint) 3);

  // Check volume calculation
  NodesT coord;
  coord <<
    15, 15,
    40, 25,
    25, 30;

  BOOST_CHECK_EQUAL(ETYPE::volume(coord), 137.5);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const ETYPE::SF::ValueT reference_result(0.1, 0.1, 0.8);
  ETYPE::SF::ValueT result;
  ETYPE::SF::compute_value(mapped_coords, result);
  cf3::Tools::Testing::Accumulator accumulator;
  cf3::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedCoordinates )
{
  const ETYPE::CoordsT test_coords(0.8, 1.2);
  const ETYPE::MappedCoordsT reference_result(1./3., 1./3.);
  ETYPE::MappedCoordsT result;
  ETYPE::compute_mapped_coordinate(test_coords, nodes, result);
  cf3::Tools::Testing::Accumulator accumulator;
  cf3::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  ConstFunctor ftor(nodes);
  cf3::Real result = 0.0;
  gauss_integrate<1, GeoShape::TRIAG>(ftor, ftor.mapped_coords, result);
  BOOST_CHECK_LT(boost::accumulators::max(cf3::Tools::Testing::test(result, ETYPE::volume(nodes)).ulps), 1);
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  ETYPE::SF::GradientT expected;
  expected(0,0) = -1.;
  expected(1,0) = -1.;
  expected(0,1) = 1.;
  expected(1,1) = 0.;
  expected(0,2) = 0.;
  expected(1,2) = 1.;
  ETYPE::SF::GradientT result;
  ETYPE::SF::compute_gradient(mapped_coords, result);
  cf3::Tools::Testing::Accumulator accumulator;
  cf3::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianDeterminant )
{
  // Shapefunction determinant should be double the volume for triangles
  BOOST_CHECK_LT(boost::accumulators::max(cf3::Tools::Testing::test(0.5*ETYPE::jacobian_determinant(mapped_coords, nodes), ETYPE::volume(nodes)).ulps), 5);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  ETYPE::JacobianT expected;
  expected(0,0) = 0.6;
  expected(0,1) = 0.9;
  expected(1,0) = 0.3;
  expected(1,1) = 1.8;
  ETYPE::JacobianT result;
  ETYPE::compute_jacobian(mapped_coords, nodes, result);
  cf3::Tools::Testing::Accumulator accumulator;
  cf3::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianAdjoint )
{
  ETYPE::JacobianT expected;
  expected(0,0) = 1.8;
  expected(0,1) = -0.9;
  expected(1,0) = -0.3;
  expected(1,1) = 0.6;
  ETYPE::JacobianT result(2, 2);
  ETYPE::compute_jacobian_adjoint(mapped_coords, nodes, result);
  cf3::Tools::Testing::Accumulator accumulator;
  cf3::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Is_coord_in_element )
{
  const ETYPE::CoordsT centroid = nodes.colwise().sum() / ETYPE::nb_nodes;

  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(centroid,nodes),true);

  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(nodes.row(0),nodes),true);
  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(nodes.row(1),nodes),true);
  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(nodes.row(2),nodes),true);

  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(centroid * 2.,nodes),false);
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
