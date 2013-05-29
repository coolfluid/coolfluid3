// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Hexa3D shape function"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/EigenAssertions.hpp"
#include <Eigen/StdVector>

#include "common/Log.hpp"

#include "common/Table.hpp"
#include "mesh/Integrators/Gauss.hpp"
#include "mesh/LagrangeP1/Hexa3D.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::mesh::Integrators;
using namespace cf3::mesh::LagrangeP1;
using namespace cf3::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Hexa3D ETYPE;

struct Hexa3DFixture
{
  /// common setup for each test case
  Hexa3DFixture() :
    mapped_coords(0.5, 0.4, 0.1),
    unit_nodes
    (
      (ETYPE::NodesT() <<
      0., 0., 0.,
      1., 0., 0.,
      1., 1., 0.,
      0., 1., 0.,
      0., 0., 1.,
      1., 0., 1.,
      1., 1., 1.,
      0., 1., 1.).finished()
    ),
    skewed_nodes
    (
      (ETYPE::NodesT() <<
      0.5, 0.5, 0.5,
      1., 0., 0.,
      1., 1., 0.,
      0., 1., 0.,
      0., 0., 1.,
      1., 0., 1.,
      1.5, 1.5, 1.5,
      0., 1., 1.).finished()
    )
  {
  }

  /// common tear-down for each test case
  ~Hexa3DFixture()
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
      return Hexa3D::jacobian_determinant(mapped_coords, m_nodes);
    }
    ETYPE::MappedCoordsT mapped_coords;
  private:
    const ETYPE::NodesT& m_nodes;
  };
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Hexa3DSuite, Hexa3DFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  BOOST_CHECK_CLOSE(ETYPE::volume(unit_nodes), 1., 0.0001);
  BOOST_CHECK_CLOSE(ETYPE::volume(skewed_nodes), 1., 0.0001);

  ETYPE::NodesT nodes_hexa3D;
  nodes_hexa3D <<
  0.0,      0.0,      0.0,
  1.0,      0.0,      0.0,
  1.0,      1.0,      0.0,
  0.0,      1.0,      0.0,
  0.0,      0.0,      1.0,
  1.0,      0.0,      1.0,
  1.0,      1.0,      1.0,
  0.0,      1.0,      1.0;
  BOOST_CHECK_EQUAL(ETYPE::volume(nodes_hexa3D), 1.);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  ETYPE::SF::ValueT reference_result;
  reference_result << 0.03375, 0.10125, 0.23625, 0.07875, 0.04125, 0.12375, 0.28875, 0.09625;

  ETYPE::SF::ValueT result;
  ETYPE::SF::compute_value(mapped_coords, result);

  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  ConstFunctor ftor(unit_nodes);
  Real result = 0.0;

  gauss_integrate<1, GeoShape::HEXA>(ftor, ftor.mapped_coords, result);
  BOOST_CHECK_CLOSE(result, 1., 0.000001);
}

BOOST_AUTO_TEST_CASE( MappedCoordinates )
{
  ETYPE::NodesT inverted;
  inverted <<
    0., 1., 1.,
    1.3, 1.4, 1.5,
    1., 0., 1.,
    0., 0., 1.,
    0., 1., 0.,
    1., 1., 0.,
    1., 0., 0.,
    0.7, 0.6, 0.5;

  const ETYPE::NodesT bignodes = unit_nodes * 100.;
  const ETYPE::NodesT biginverted = inverted * 100.;

  ETYPE::NodesT parallelepiped = unit_nodes;
  parallelepiped.block<4, 1>(4, XX).array() += 1.;
  parallelepiped.block<4, 1>(4, YY).array() += 0.3;

  const Real max_ulps = boost::accumulators::max(test(0.1+1e-12, 0.1).ulps); //expected max ulps based on the iteration threshold

  // boost::assign doesn't work here, because of the alignment issues with Eigen fixed-size types
  std::vector<ETYPE::NodesT,Eigen::aligned_allocator<ETYPE::NodesT> > test_nodes;
  test_nodes.push_back(unit_nodes);
  test_nodes.push_back(skewed_nodes);
  test_nodes.push_back(inverted);
  test_nodes.push_back(bignodes);
  test_nodes.push_back(biginverted);
  test_nodes.push_back(parallelepiped);
  const std::vector<Real> ulps_list = boost::assign::list_of(15.)(max_ulps)(max_ulps)(15.)(max_ulps)(20.);
  Uint idx = 0;
  BOOST_FOREACH(const ETYPE::NodesT& nodes, test_nodes)
  {
    ETYPE::SF::ValueT sf;
    ETYPE::SF::compute_value(mapped_coords, sf);
    const ETYPE::CoordsT coords = sf * nodes;
    std::cout << "Looking for coords " << coords << " in mapped coords " << mapped_coords << std::endl;

    ETYPE::MappedCoordsT result;
    ETYPE::compute_mapped_coordinate(coords, nodes, result);

    Accumulator accumulator;
    vector_test(result, mapped_coords, accumulator);
    BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), ulps_list[idx++]);
  }
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  ETYPE::SF::GradientT expected;
  expected(KSI, 0) = -0.06750;
  expected(ETA, 0) = -0.05625;
  expected(ZTA, 0) = -0.03750;
  expected(KSI, 1) = 0.06750;
  expected(ETA, 1) = -0.16875;
  expected(ZTA, 1) = -0.1125;
  expected(KSI, 2) = 0.1575;
  expected(ETA, 2) = 0.16875;
  expected(ZTA, 2) = -0.2625;
  expected(KSI, 3) = -0.1575;
  expected(ETA, 3) = 0.05625;
  expected(ZTA, 3) = -0.08750;
  expected(KSI, 4) = -0.08250;
  expected(ETA, 4) = -0.06875;
  expected(ZTA, 4) = 0.03750;
  expected(KSI, 5) = 0.08250;
  expected(ETA, 5) = -0.20625;
  expected(ZTA, 5) = 0.1125;
  expected(KSI, 6) = 0.1925;
  expected(ETA, 6) = 0.20625;
  expected(ZTA, 6) = 0.2625;
  expected(KSI, 7) = -0.1925;
  expected(ETA, 7) = 0.06875;
  expected(ZTA, 7) = 0.08750;

  ETYPE::SF::GradientT result;
  ETYPE::SF::compute_gradient(mapped_coords, result);

  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianDeterminant )
{
  BOOST_CHECK_CLOSE(ETYPE::jacobian_determinant(mapped_coords, unit_nodes), 0.125, 0.00001);
  BOOST_CHECK_CLOSE(ETYPE::jacobian_determinant(mapped_coords, skewed_nodes), 0.1875, 0.00001);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  ETYPE::JacobianT expected;
  expected(0,0) = 0.5625;
  expected(0,1) = 0.06250;
  expected(0,2) = 0.06250;
  expected(1,0) = 0.07500;
  expected(1,1) = 0.5750;
  expected(1,2) = 0.07500;
  expected(2,0) = 0.1125;
  expected(2,1) = 0.1125;
  expected(2,2) = 0.6125;

  ETYPE::JacobianT result;
  ETYPE::compute_jacobian(mapped_coords, skewed_nodes, result);

  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianAdjoint )
{
  ETYPE::JacobianT expected;
  expected(0,0) = 0.34375;
  expected(0,1) = -0.03125;
  expected(0,2) = -0.03125;
  expected(1,0) = -0.03750;
  expected(1,1) = 0.3375;
  expected(1,2) = -0.03750;
  expected(2,0) = -0.05625;
  expected(2,1) = -0.05625;
  expected(2,2) = 0.31875;

  ETYPE::JacobianT result;
  ETYPE::compute_jacobian_adjoint(mapped_coords, skewed_nodes, result);

  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Is_coord_in_element )
{
  const ETYPE::CoordsT centroid = skewed_nodes.colwise().sum() / ETYPE::nb_nodes;

  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(centroid,skewed_nodes),true);

  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(skewed_nodes.row(0),skewed_nodes),true);
  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(skewed_nodes.row(2),skewed_nodes),true);
  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(skewed_nodes.row(5),skewed_nodes),true);
  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(skewed_nodes.row(6),skewed_nodes),true);
  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(skewed_nodes.row(7),skewed_nodes),true);

  BOOST_CHECK_EQUAL(ETYPE::is_coord_in_element(centroid*5.,skewed_nodes),false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

