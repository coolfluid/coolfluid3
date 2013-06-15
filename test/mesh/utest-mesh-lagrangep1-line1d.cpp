// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Line1D shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"

#include "mesh/ElementType.hpp"
#include "common/Table.hpp"
#include "mesh/Integrators/Gauss.hpp"
#include "mesh/LagrangeP1/Line1D.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::Integrators;
using namespace cf3::mesh::LagrangeP1;
using namespace cf3::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Line1D ETYPE;

struct LagrangeP1Line1DFixture
{
  /// common setup for each test case
  LagrangeP1Line1DFixture() :
    mapped_coords((ETYPE::MappedCoordsT() << .2).finished()),
    nodes((ETYPE::NodesT() << 5., 10.).finished())
  {
  }

  /// common tear-down for each test case
  ~LagrangeP1Line1DFixture()
  {
  }
  /// common values accessed by all tests goes here

  const ETYPE::MappedCoordsT mapped_coords;
  const ETYPE::NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const ETYPE::NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      return Line1D::jacobian_determinant(mapped_coords, m_nodes);
    }
    ETYPE::MappedCoordsT mapped_coords;
  private:
    const ETYPE::NodesT& m_nodes;
  };
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Line1DSuite, LagrangeP1Line1DFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  RealMatrix nodes_line1D ( (RealMatrix(2,1) << 2.0, 1.0).finished() );
  boost::shared_ptr< ElementType > line1d = build_component_abstract_type<ElementType>("cf3.mesh.LagrangeP1.Line1D","line1d");
  BOOST_CHECK_EQUAL(line1d->volume(nodes_line1D),1.);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const ETYPE::SF::ValueT reference_result(0.4, 0.6);
  ETYPE::SF::ValueT result;
  Line1D::SF::compute_value(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedCoordinates )
{
  ETYPE::CoordsT test_coords;
  test_coords << 6.;
  ETYPE::MappedCoordsT result;
  Line1D::compute_mapped_coordinate(test_coords, nodes, result);
  Accumulator acc = test(result[0], -0.6);
  BOOST_CHECK_LT(boost::accumulators::max(acc.ulps), 5);
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  ETYPE::SF::GradientT result;
  ETYPE::SF::GradientT expected;
  expected(0,0) = -0.5;
  expected(0,1) = 0.5;
  Line1D::SF::compute_gradient(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianDeterminant )
{
  BOOST_CHECK_LT(boost::accumulators::max(test(Line1D::jacobian_determinant(mapped_coords, nodes), 0.5*Line1D::volume(nodes)).ulps), 1);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  ETYPE::JacobianT expected;
  expected << 2.5;
  ETYPE::JacobianT result;
  Line1D::compute_jacobian(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianAdjoint )
{
  ETYPE::JacobianT expected;
  expected << 1.;
  ETYPE::JacobianT result;
  Line1D::compute_jacobian_adjoint(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 1);
}

BOOST_AUTO_TEST_CASE( integrateConst )
{
  ConstFunctor ftor(nodes);
  const Real vol = Line1D::volume(nodes);

  cf3::Real result1 = 0.0;
  cf3::Real result2 = 0.0;
  cf3::Real result4 = 0.0;
  cf3::Real result8 = 0.0;
  cf3::Real result16 = 0.0;
  cf3::Real result32 = 0.0;

  gauss_integrate<1, GeoShape::LINE>(ftor, ftor.mapped_coords, result1);
  gauss_integrate<2, GeoShape::LINE>(ftor, ftor.mapped_coords, result2);
  gauss_integrate<4, GeoShape::LINE>(ftor, ftor.mapped_coords, result4);
  gauss_integrate<8, GeoShape::LINE>(ftor, ftor.mapped_coords, result8);
  gauss_integrate<16, GeoShape::LINE>(ftor, ftor.mapped_coords, result16);
  gauss_integrate<32, GeoShape::LINE>(ftor, ftor.mapped_coords, result32);

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

