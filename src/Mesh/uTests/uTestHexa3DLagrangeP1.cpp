// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Hexa3DLagrangeP1 shape function"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CArray.hpp"
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

struct Hexa3DLagrangeP1Fixture
{
  typedef std::vector<RealVector> NodesT;
  /// common setup for each test case
  Hexa3DLagrangeP1Fixture() : mapped_coords(init_mapped_coords()), unit_nodes(init_nodes_unit_cube()), skewed_nodes(init_nodes_skewed_cube())
  {
  }

  /// common tear-down for each test case
  ~Hexa3DLagrangeP1Fixture()
  {
  }
  /// common values accessed by all tests goes here

  const RealVector mapped_coords;
  const NodesT unit_nodes;
  const NodesT skewed_nodes;

  struct const_functor
  {
    const_functor(const NodesT& node_list) : m_nodes(node_list) {}
    template<typename GeoShapeF, typename SolShapeF>
    Real valTimesDetJacobian(const RealVector& mappedCoords)
    {
      return GeoShapeF::jacobian_determinant(mappedCoords, m_nodes);
    }
  private:
    const NodesT& m_nodes;
  };

private:
  /// Workaround for boost:assign ambiguity
  RealVector init_mapped_coords()
  {
    return list_of(0.5)(0.4)(0.1);
  }

  /// Workaround for boost:assign ambiguity
  NodesT init_nodes_unit_cube()
  {
    const RealVector c0 = list_of(0.)(0.)(0.);
    const RealVector c1 = list_of(1.)(0.)(0.);
    const RealVector c2 = list_of(1.)(1.)(0.);
    const RealVector c3 = list_of(0.)(1.)(0.);
    const RealVector c4 = list_of(0.)(0.)(1.);
    const RealVector c5 = list_of(1.)(0.)(1.);
    const RealVector c6 = list_of(1.)(1.)(1.);
    const RealVector c7 = list_of(0.)(1.)(1.);

    return list_of(c0)(c1)(c2)(c3)(c4)(c5)(c6)(c7);
  }

  NodesT init_nodes_skewed_cube()
  {
    const RealVector c0 = list_of(0.5)(0.5)(0.5);
    const RealVector c1 = list_of(1.)(0.)(0.);
    const RealVector c2 = list_of(1.)(1.)(0.);
    const RealVector c3 = list_of(0.)(1.)(0.);
    const RealVector c4 = list_of(0.)(0.)(1.);
    const RealVector c5 = list_of(1.)(0.)(1.);
    const RealVector c6 = list_of(1.5)(1.5)(1.5);
    const RealVector c7 = list_of(0.)(1.)(1.);

    return list_of(c0)(c1)(c2)(c3)(c4)(c5)(c6)(c7);
  }

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Hexa3DLagrangeP1Suite, Hexa3DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  BOOST_CHECK_CLOSE(Hexa3DLagrangeP1::volume(unit_nodes), 1., 0.0001);
  BOOST_CHECK_CLOSE(Hexa3DLagrangeP1::volume(skewed_nodes), 1., 0.0001);
  
  boost::multi_array<Real,2> nodes_hexa3D (boost::extents[8][3]);
  nodes_hexa3D[0][XX] = 0.0;     nodes_hexa3D[0][YY] = 0.0;     nodes_hexa3D[0][ZZ] = 0.0;
  nodes_hexa3D[1][XX] = 1.0;     nodes_hexa3D[1][YY] = 0.0;     nodes_hexa3D[1][ZZ] = 0.0;
  nodes_hexa3D[2][XX] = 1.0;     nodes_hexa3D[2][YY] = 1.0;     nodes_hexa3D[2][ZZ] = 0.0;
  nodes_hexa3D[3][XX] = 0.0;     nodes_hexa3D[3][YY] = 1.0;     nodes_hexa3D[3][ZZ] = 0.0;
  nodes_hexa3D[4][XX] = 0.0;     nodes_hexa3D[4][YY] = 0.0;     nodes_hexa3D[4][ZZ] = 1.0;
  nodes_hexa3D[5][XX] = 1.0;     nodes_hexa3D[5][YY] = 0.0;     nodes_hexa3D[5][ZZ] = 1.0;
  nodes_hexa3D[6][XX] = 1.0;     nodes_hexa3D[6][YY] = 1.0;     nodes_hexa3D[6][ZZ] = 1.0;
  nodes_hexa3D[7][XX] = 0.0;     nodes_hexa3D[7][YY] = 1.0;     nodes_hexa3D[7][ZZ] = 1.0;
  BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::volume(nodes_hexa3D), 1.);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const RealVector reference_result = list_of(0.03375)(0.10125)(0.23625)(0.07875)(0.04125)(0.12375)(0.28875)(0.09625);

  RealVector result(8);
  Hexa3DLagrangeP1::shape_function(mapped_coords, result);

  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  const_functor ftor(unit_nodes);
  Real result = 0.0;

  Gauss<Hexa3DLagrangeP1>::integrateElement(ftor, result);
  BOOST_CHECK_CLOSE(result, 1., 0.000001);
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  RealMatrix expected(Hexa3DLagrangeP1::dimension, Hexa3DLagrangeP1::nb_nodes);
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

  RealMatrix result(Hexa3DLagrangeP1::dimension, Hexa3DLagrangeP1::nb_nodes);
  Hexa3DLagrangeP1::mapped_gradient(mapped_coords, result);

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
  RealMatrix expected(3, 3);
  expected(0,0) = 0.5625;
  expected(0,1) = 0.06250;
  expected(0,2) = 0.06250;
  expected(1,0) = 0.07500;
  expected(1,1) = 0.5750;
  expected(1,2) = 0.07500;
  expected(2,0) = 0.1125;
  expected(2,1) = 0.1125;
  expected(2,2) = 0.6125;

  RealMatrix result(3, 3);
  Hexa3DLagrangeP1::jacobian(mapped_coords, skewed_nodes, result);

  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianAdjoint )
{
  RealMatrix expected(3, 3);
  expected(0,0) = 0.34375;
  expected(0,1) = -0.03125;
  expected(0,2) = -0.03125;
  expected(1,0) = -0.03750;
  expected(1,1) = 0.3375;
  expected(1,2) = -0.03750;
  expected(2,0) = -0.05625;
  expected(2,1) = -0.05625;
  expected(2,2) = 0.31875;

  RealMatrix result(3, 3);
  Hexa3DLagrangeP1::jacobian_adjoint(mapped_coords, skewed_nodes, result);

  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Is_coord_in_element )
{
  RealVector centroid(3);
  for (Uint i=0; i<Hexa3DLagrangeP1::nb_nodes; ++i)
		centroid += skewed_nodes[i];
	centroid /= Hexa3DLagrangeP1::nb_nodes;

  BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(centroid,skewed_nodes),true);
	
	BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(skewed_nodes[0],skewed_nodes),true);
	BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(skewed_nodes[2],skewed_nodes),true);
	BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(skewed_nodes[5],skewed_nodes),true);
	BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(skewed_nodes[6],skewed_nodes),true);
	BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(skewed_nodes[7],skewed_nodes),true);
	
	
	RealVector outside_coord(3);
	outside_coord = 5.0*centroid;
	BOOST_CHECK_EQUAL(Hexa3DLagrangeP1::in_element(outside_coord,skewed_nodes),false);
	
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

