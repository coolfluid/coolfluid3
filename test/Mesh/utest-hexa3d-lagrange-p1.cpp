// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Hexa3DLagrangeP1 shape function"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include <Eigen/StdVector>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CTable.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Hexa3DLagrangeP1.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::SF;
using namespace CF::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Hexa3DLagrangeP1 SFT;

struct Hexa3DLagrangeP1Fixture
{
  /// common setup for each test case
  Hexa3DLagrangeP1Fixture() :
    mapped_coords(0.5, 0.4, 0.1),
    unit_nodes
    (
      (SFT::NodeMatrixT() <<
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
      (SFT::NodeMatrixT() <<
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
  ~Hexa3DLagrangeP1Fixture()
  {
  }
  /// common values accessed by all tests goes here
  const SFT::MappedCoordsT mapped_coords;
  const SFT::NodeMatrixT unit_nodes;
  const SFT::NodeMatrixT skewed_nodes;

  struct ConstFunctor
  {
    ConstFunctor(const SFT::NodeMatrixT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      return Hexa3DLagrangeP1::jacobian_determinant(mapped_coords, m_nodes);
    }
    SFT::MappedCoordsT mapped_coords;
  private:
    const SFT::NodeMatrixT& m_nodes;
  };
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Hexa3DLagrangeP1Suite, Hexa3DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  BOOST_CHECK_CLOSE(Hexa3DLagrangeP1::volume(unit_nodes), 1., 0.0001);
  BOOST_CHECK_CLOSE(Hexa3DLagrangeP1::volume(skewed_nodes), 1., 0.0001);
  
  SFT::NodeMatrixT nodes_hexa3D;
  nodes_hexa3D <<
  0.0,      0.0,      0.0, 
  1.0,      0.0,      0.0, 
  1.0,      1.0,      0.0, 
  0.0,      1.0,      0.0, 
  0.0,      0.0,      1.0, 
  1.0,      0.0,      1.0, 
  1.0,      1.0,      1.0, 
  0.0,      1.0,      1.0;
  BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::volume(nodes_hexa3D), 1.);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  SFT::ShapeFunctionsT reference_result;
  reference_result << 0.03375, 0.10125, 0.23625, 0.07875, 0.04125, 0.12375, 0.28875, 0.09625;

  SFT::ShapeFunctionsT result;
  Hexa3DLagrangeP1::shape_function_value(mapped_coords, result);

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
  SFT::NodeMatrixT inverted;
  inverted <<
    0., 1., 1.,
    1.3, 1.4, 1.5,
    1., 0., 1.,
    0., 0., 1.,
    0., 1., 0.,
    1., 1., 0.,
    1., 0., 0.,
    0.7, 0.6, 0.5;
    
  const SFT::NodeMatrixT bignodes = unit_nodes * 100.;
  const SFT::NodeMatrixT biginverted = inverted * 100.;
    
  SFT::NodeMatrixT parallelepiped = unit_nodes;
  parallelepiped.block<4, 1>(4, XX).array() += 1.;
  parallelepiped.block<4, 1>(4, YY).array() += 0.3;
  
  const Real max_ulps = boost::accumulators::max(test(0.1+1e-12, 0.1).ulps); //expected max ulps based on the iteration threshold
  
  // boost::assign doesn't work here, because of the alignment issues with Eigen fixed-size types
  std::vector<SFT::NodeMatrixT,Eigen::aligned_allocator<SFT::NodeMatrixT> > test_nodes;
  test_nodes.push_back(unit_nodes);
  test_nodes.push_back(skewed_nodes);
  test_nodes.push_back(inverted);
  test_nodes.push_back(bignodes);
  test_nodes.push_back(biginverted);
  test_nodes.push_back(parallelepiped);
  const std::vector<Real> ulps_list = boost::assign::list_of(15.)(max_ulps)(max_ulps)(15.)(max_ulps)(20.);
  Uint idx = 0;
  BOOST_FOREACH(const SFT::NodeMatrixT& nodes, test_nodes)
  {
    SFT::ShapeFunctionsT sf;
    SFT::shape_function_value(mapped_coords, sf);
    const SFT::CoordsT coords = sf * nodes;
    std::cout << "Looking for coords " << coords << " in mapped coords " << mapped_coords << std::endl;
    
    SFT::MappedCoordsT result;
    Hexa3DLagrangeP1::mapped_coordinates(coords, nodes, result);
    
    Accumulator accumulator;
    vector_test(result, mapped_coords, accumulator);
    BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), ulps_list[idx++]);
  }
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  SFT::MappedGradientT expected;
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

  SFT::MappedGradientT result;
  Hexa3DLagrangeP1::shape_function_gradient(mapped_coords, result);

  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianDeterminant )
{
  BOOST_CHECK_CLOSE(Hexa3DLagrangeP1::jacobian_determinant(mapped_coords, unit_nodes), 0.125, 0.00001);
  BOOST_CHECK_CLOSE(Hexa3DLagrangeP1::jacobian_determinant(mapped_coords, skewed_nodes), 0.1875, 0.00001);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  SFT::JacobianT expected;
  expected(0,0) = 0.5625;
  expected(0,1) = 0.06250;
  expected(0,2) = 0.06250;
  expected(1,0) = 0.07500;
  expected(1,1) = 0.5750;
  expected(1,2) = 0.07500;
  expected(2,0) = 0.1125;
  expected(2,1) = 0.1125;
  expected(2,2) = 0.6125;

  SFT::JacobianT result;
  Hexa3DLagrangeP1::jacobian(mapped_coords, skewed_nodes, result);

  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianAdjoint )
{
  SFT::JacobianT expected;
  expected(0,0) = 0.34375;
  expected(0,1) = -0.03125;
  expected(0,2) = -0.03125;
  expected(1,0) = -0.03750;
  expected(1,1) = 0.3375;
  expected(1,2) = -0.03750;
  expected(2,0) = -0.05625;
  expected(2,1) = -0.05625;
  expected(2,2) = 0.31875;

  SFT::JacobianT result;
  Hexa3DLagrangeP1::jacobian_adjoint(mapped_coords, skewed_nodes, result);

  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Is_coord_in_element )
{
  const SFT::CoordsT centroid = skewed_nodes.colwise().sum() / SFT::nb_nodes;

  BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(centroid,skewed_nodes),true);
  
  BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(skewed_nodes.row(0),skewed_nodes),true);
  BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(skewed_nodes.row(2),skewed_nodes),true);
  BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(skewed_nodes.row(5),skewed_nodes),true);
  BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(skewed_nodes.row(6),skewed_nodes),true);
  BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(skewed_nodes.row(7),skewed_nodes),true);
  
  BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(centroid*5.,skewed_nodes),false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

