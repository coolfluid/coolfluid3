// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Prism3D shape function"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/EigenAssertions.hpp"
#include <Eigen/StdVector>

#include "common/Log.hpp"

#include "common/Table.hpp"
#include "mesh/Integrators/Gauss.hpp"
#include "mesh/LagrangeP1/Prism3D.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::mesh::Integrators;
using namespace cf3::mesh::LagrangeP1;
using namespace cf3::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Prism3D ETYPE;

struct Prism3DFixture
{
  /// common setup for each test case
  Prism3DFixture() :
    mapped_coords(0.5, 0.4, 0.1),
    unit_nodes
    (
      (ETYPE::NodesT() <<
      0., 0., 0.,
      1., 0., 0.,
      0., 1., 0.,
      0., 0., 1.,
      1., 0., 1.,
      0., 1., 1.).finished()
    ),
    skewed_nodes
    (
      (ETYPE::NodesT() <<
       0.2, 0.3, 0.,
       1.2, 0.3, 0.,
       0.2, 1.3, 0.,
       0.2, 0.3, 1.,
       1.2, 0.3, 1.,
       0.2, 1.3, 1.).finished()
    )
  {
  }

  /// common tear-down for each test case
  ~Prism3DFixture()
  {
  }
  /// common values accessed by all tests goes here
  const ETYPE::MappedCoordsT mapped_coords;
  const ETYPE::NodesT unit_nodes;
  const ETYPE::NodesT skewed_nodes;

  struct ConstFunctor
  {
    ConstFunctor(const ETYPE::NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      return Prism3D::jacobian_determinant(mapped_coords, m_nodes);
    }
    ETYPE::MappedCoordsT mapped_coords;
  private:
    const ETYPE::NodesT& m_nodes;
  };
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Prism3DSuite, Prism3DFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  BOOST_CHECK_CLOSE(ETYPE::volume(unit_nodes), 0.5, 0.0001);
  BOOST_CHECK_CLOSE(ETYPE::volume(skewed_nodes), 0.5, 0.0001);

  ETYPE::NodesT nodes_Prism3D;
  nodes_Prism3D <<
  0.0,      0.0,      0.0,
  1.0,      0.0,      0.0,
  0.0,      1.0,      0.0,
  0.0,      0.0,      1.0,
  1.0,      0.0,      1.0,
  0.0,      1.0,      1.0;
  BOOST_CHECK_EQUAL(ETYPE::volume(nodes_Prism3D), 0.5);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  ETYPE::SF::ValueT reference_result;
  reference_result << 0.045, 0.225, 0.18, 0.055, 0.275, 0.22;

  ETYPE::SF::ValueT result;
  ETYPE::SF::compute_value(mapped_coords, result);

  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  ConstFunctor ftor(skewed_nodes);
  Real result = 0.0;

  gauss_integrate<1, GeoShape::PRISM>(ftor, ftor.mapped_coords, result);
  BOOST_CHECK_CLOSE(result, 0.5, 0.000001);

  gauss_integrate<2, GeoShape::PRISM>(ftor, ftor.mapped_coords, result);
  BOOST_CHECK_CLOSE(result, 0.5, 0.000001);
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  ETYPE::SF::GradientT expected;
  expected << -0.45,  0.45,    0, -0.55, 0.55,    0,
      -0.45,     0, 0.45, -0.55,    0, 0.55,
      -0.05, -0.25, -0.2,  0.05, 0.25,  0.2;

  ETYPE::SF::GradientT result;
  ETYPE::SF::compute_gradient(mapped_coords, result);

  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 3);
}

BOOST_AUTO_TEST_CASE( JacobianDeterminant )
{
  BOOST_CHECK_CLOSE(ETYPE::jacobian_determinant(mapped_coords, unit_nodes), 0.5, 0.00001);
  BOOST_CHECK_CLOSE(ETYPE::jacobian_determinant(mapped_coords, skewed_nodes), 0.5, 0.00001);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  ETYPE::JacobianT expected;
  expected << 1, 0, 0, 0, 1, 0, 0, 0, 0.5;

  ETYPE::JacobianT result;
  ETYPE::compute_jacobian(mapped_coords, skewed_nodes, result);

  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianAdjoint )
{
  ETYPE::JacobianT expected;
  expected << 0.5, 0, 0, 0, 0.5, 0, 0, 0, 1.;

  ETYPE::JacobianT result;
  ETYPE::compute_jacobian_adjoint(mapped_coords, skewed_nodes, result);

  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

