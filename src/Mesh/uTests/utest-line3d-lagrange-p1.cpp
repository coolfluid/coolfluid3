// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Line3DLagrangeP1 shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Math/MathConsts.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Line3DLagrangeP1.hpp"



#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Math;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::SF;
using namespace CF::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Line3DLagrangeP1 SFT;

struct LagrangeSFLine3DLagrangeP1Fixture
{
  typedef SFT::NodeMatrixT NodesT;
  /// common setup for each test case
  LagrangeSFLine3DLagrangeP1Fixture() : mapped_coords((SFT::MappedCoordsT() << .2).finished()), nodes((NodesT() << 5., 7., 2.,
                                                                              10., 3., 3.).finished())
  {
  }

  /// common tear-down for each test case
  ~LagrangeSFLine3DLagrangeP1Fixture()
  {
  }

  /// Fills the given coordinate and connectivity data to create a helix along the Z-axis, consisting of Line3DLagrangeP1 elements
  void create_helix(CArray& coordinates, CTable& connectivity, const Real radius, const Real height, const Real tours, const Uint segments)
  {
    const Uint dim = Line3DLagrangeP1::dimension;
    const Uint nb_nodes = Line3DLagrangeP1::nb_nodes;
    const Real start_angle = 0.;
    const Real end_angle = tours*2.*MathConsts::RealPi;

    coordinates.initialize(dim);
    CArray::ArrayT& coord_array = coordinates.array();
    coord_array.resize(boost::extents[segments + 1][dim]);

    connectivity.initialize(nb_nodes);
    CTable::ArrayT& conn_array = connectivity.array();
    conn_array.resize(boost::extents[segments][nb_nodes]);
    const Real height_step = height / segments;
    for(Uint u = 0; u != segments; ++u)
    {
      const Real theta = start_angle + (end_angle - start_angle) * (static_cast<Real>(u) / static_cast<Real>(segments));
      CArray::Row coord_row = coord_array[u];

      coord_row[XX] = radius * cos(theta);
      coord_row[YY] = radius * sin(theta);
      coord_row[ZZ] = u*height_step;

      CTable::Row nodes = conn_array[u];
      nodes[0] = u;
      nodes[1] = u+1;
    }
    CArray::Row coord_row = coord_array[segments];
    coord_row[XX] = radius * cos(end_angle);
    coord_row[YY] = radius * sin(end_angle);
    coord_row[ZZ] = segments * height_step;
  }

  const SFT::MappedCoordsT mapped_coords;
  const NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      SFT::JacobianT jac;
      SFT::jacobian(mapped_coords, m_nodes, jac);
      return jac.norm();
    }
    SFT::MappedCoordsT mapped_coords;
  private:
    const NodesT& m_nodes;
  };

  /// Returns the norm of the tangen vector to the curve
  struct TangentVectorNorm {

    template<typename NodesT>
    Real operator()(const SFT::MappedCoordsT& mapped_coords, const NodesT& nodes)
    {
      SFT::JacobianT jac;
      SFT::jacobian(mapped_coords, nodes, jac);
      Real result = 0.;
      for(Uint i = 0; i != SFT::dimension; ++i)
      {
        result += jac(0, i) * jac(0, i);
      }
      return sqrt(result);
    }
  };

  Real square(const Real a)
  {
    return a*a;
  }
};

/// Integral over a region
template<typename ResultT, typename FunctorT>
void integrate_region(ResultT& result, FunctorT functor, const CArray& coordinates, const CTable& connectivity)
{
  const Uint nb_elems = connectivity.array().size();
  for(Uint elem_idx = 0; elem_idx != nb_elems; ++ elem_idx)
  {
    SFT::NodeMatrixT nodes;
    fill(nodes, coordinates, connectivity.array()[elem_idx]);
    integrate_element(result, functor, nodes);
  }
}

/// Integral over an element
template<typename ResultT, typename FunctorT, typename NodesT>
void integrate_element(ResultT& result, FunctorT functor, const NodesT& nodes)
{
  static const double mu = 0.;
  static const double w = 2.;
  static const SFT::MappedCoordsT mapped_coords((SFT::MappedCoordsT() << mu).finished());
  result += w * functor(mapped_coords, nodes);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Line3DLagrangeP1Suite, LagrangeSFLine3DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Length )
{
  NodesT nodes_line3D;
  nodes_line3D << 2., 2., 2.,
                  1., 1., 1.;
  BOOST_CHECK_EQUAL(Line3DLagrangeP1::length(nodes_line3D),std::sqrt(3.));
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  SFT::ShapeFunctionsT reference_result;
  reference_result << 0.4, 0.6;
  SFT::ShapeFunctionsT result;
  Line3DLagrangeP1::shape_function(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  SFT::MappedGradientT result;
  SFT::MappedGradientT expected(-0.5, 0.5);
  Line3DLagrangeP1::mapped_gradient(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  SFT::JacobianT expected(2.5, -2., 0.5);
  SFT::JacobianT result;
  Line3DLagrangeP1::jacobian(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  ConstFunctor ftor(nodes);
  const Real len = Line3DLagrangeP1::length(nodes);

  Real result1 = 0.0;
  Real result2 = 0.0;
  Real result4 = 0.0;
  Real result8 = 0.0;
  Real result16 = 0.0;
  Real result32 = 0.0;

  gauss_integrate<1, GeoShape::LINE>(ftor, ftor.mapped_coords, result1);
  gauss_integrate<2, GeoShape::LINE>(ftor, ftor.mapped_coords, result2);
  gauss_integrate<4, GeoShape::LINE>(ftor, ftor.mapped_coords, result4);
  gauss_integrate<8, GeoShape::LINE>(ftor, ftor.mapped_coords, result8);
  gauss_integrate<16, GeoShape::LINE>(ftor, ftor.mapped_coords, result16);
  gauss_integrate<32, GeoShape::LINE>(ftor, ftor.mapped_coords, result32);

  BOOST_CHECK_CLOSE(result1, len, 0.00001);
  BOOST_CHECK_CLOSE(result2, len, 0.00001);
  BOOST_CHECK_CLOSE(result4, len, 0.00001);
  BOOST_CHECK_CLOSE(result8, len, 0.00001);
  BOOST_CHECK_CLOSE(result16, len, 0.00001);
  BOOST_CHECK_CLOSE(result32, len, 0.00001);
}

BOOST_AUTO_TEST_CASE( LineIntegral )
{
  // Create an approximation of a helix
  const Real radius = 1.;
  const Real height = 5.;
  const Real tours = 3.;
  const Uint segments = 10000;

  // complete circle
  CArray coordinates("coordinates");
  CTable connectivity("connectivity");
  create_helix(coordinates, connectivity, radius, height, tours, segments);

  // Check the length, using the line integral of one times the norm of the tangent vector
  Real length = 0.;
  integrate_region(length, TangentVectorNorm(), coordinates, connectivity);
  BOOST_CHECK_CLOSE(length, tours*sqrt((square(2.*MathConsts::RealPi*radius)+square(height/tours))), 0.01);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

