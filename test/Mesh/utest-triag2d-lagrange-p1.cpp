// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Triag2DLagrangeP1"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CNodes.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Triag2DLagrangeP1.hpp"
#include "Mesh/CElements.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::SF;

//////////////////////////////////////////////////////////////////////////////

typedef Triag2DLagrangeP1 SFT;

struct Triag2DLagrangeP1Fixture
{
  typedef SFT::NodeMatrixT NodesT;
  /// common setup for each test case
  Triag2DLagrangeP1Fixture() : mapped_coords(0.1, 0.8), nodes((NodesT() << 0.5, 0.3,
                                                                           1.1, 1.2,
                                                                           0.8, 2.1).finished())
  {
  }

  /// common tear-down for each test case
  ~Triag2DLagrangeP1Fixture()
  {
  }
  /// common values accessed by all tests goes here

  const SFT::MappedCoordsT mapped_coords;
  const NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      return Triag2DLagrangeP1::jacobian_determinant(mapped_coords, m_nodes);
    }
    SFT::MappedCoordsT mapped_coords;
  private:
    const NodesT& m_nodes;
  };
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Triag2DLagrangeP1Suite, Triag2DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  NodesT nodes_triag2D;
  nodes_triag2D <<
    0.0, 0.0, 
    1.0, 0.0, 
    1.0, 1.0;
  BOOST_CHECK_EQUAL(Triag2DLagrangeP1::volume(nodes_triag2D), 0.5);
}

BOOST_AUTO_TEST_CASE( Element )
{
  CNodes::Ptr nodes = allocate_component<CNodes>("nodes") ;
  // Create a CElements component
  CElements::Ptr comp = allocate_component<CElements>("comp");

  comp->initialize("CF.Mesh.SF.Triag2DLagrangeP1",*nodes);
  BOOST_CHECK_EQUAL(comp->element_type().shape(), GeoShape::TRIAG);
  BOOST_CHECK_EQUAL(comp->element_type().nb_nodes(), (Uint) 3);

  // Check volume calculation
  NodesT coord;
  coord <<
    15, 15, 
    40, 25, 
    25, 30;
  
  BOOST_CHECK_EQUAL(Triag2DLagrangeP1::volume(coord), 137.5);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const SFT::ShapeFunctionsT reference_result(0.1, 0.1, 0.8);
  SFT::ShapeFunctionsT result;
  Triag2DLagrangeP1::shape_function_value(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedCoordinates )
{
  const SFT::CoordsT test_coords(0.8, 1.2);
  const SFT::MappedCoordsT reference_result(1./3., 1./3.);
  SFT::MappedCoordsT result;
  Triag2DLagrangeP1::mapped_coordinates(test_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  ConstFunctor ftor(nodes);
  CF::Real result = 0.0;
  gauss_integrate<1, GeoShape::TRIAG>(ftor, ftor.mapped_coords, result);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(result, Triag2DLagrangeP1::volume(nodes)).ulps), 1);
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  SFT::MappedGradientT expected;
  expected(0,0) = -1.;
  expected(1,0) = -1.;
  expected(0,1) = 1.;
  expected(1,1) = 0.;
  expected(0,2) = 0.;
  expected(1,2) = 1.;
  SFT::MappedGradientT result;
  Triag2DLagrangeP1::shape_function_gradient(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianDeterminant )
{
  // Shapefunction determinant should be double the volume for triangles
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(0.5*Triag2DLagrangeP1::jacobian_determinant(mapped_coords, nodes), Triag2DLagrangeP1::volume(nodes)).ulps), 5);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  SFT::JacobianT expected;
  expected(0,0) = 0.6;
  expected(0,1) = 0.9;
  expected(1,0) = 0.3;
  expected(1,1) = 1.8;
  SFT::JacobianT result;
  Triag2DLagrangeP1::jacobian(mapped_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianAdjoint )
{
  SFT::JacobianT expected;
  expected(0,0) = 1.8;
  expected(0,1) = -0.9;
  expected(1,0) = -0.3;
  expected(1,1) = 0.6;
  SFT::JacobianT result(2, 2);
  Triag2DLagrangeP1::jacobian_adjoint(mapped_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Is_coord_in_element )
{
  const SFT::CoordsT centroid = nodes.colwise().sum() / SFT::nb_nodes;
  
  BOOST_CHECK_EQUAL(Triag2DLagrangeP1::in_element(centroid,nodes),true);
  
  BOOST_CHECK_EQUAL(Triag2DLagrangeP1::in_element(nodes.row(0),nodes),true);
  BOOST_CHECK_EQUAL(Triag2DLagrangeP1::in_element(nodes.row(1),nodes),true);
  BOOST_CHECK_EQUAL(Triag2DLagrangeP1::in_element(nodes.row(2),nodes),true);	
  
  BOOST_CHECK_EQUAL(Triag2DLagrangeP1::in_element(centroid * 2.,nodes),false);
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
