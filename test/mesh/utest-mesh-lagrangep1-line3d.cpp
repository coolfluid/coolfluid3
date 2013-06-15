// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the LagrangeP1 Line3D Element types"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"

#include "math/Consts.hpp"

#include "mesh/ElementType.hpp"
#include "common/Table.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Integrators/Gauss.hpp"
#include "mesh/LagrangeP1/Line3D.hpp"



#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::math;
using namespace cf3::mesh;
using namespace cf3::mesh::Integrators;
using namespace cf3::mesh::LagrangeP1;
using namespace cf3::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Line3D ETYPE;

struct LagrangeP1Line3DFixture
{
  typedef ETYPE::NodesT NodesT;
  /// common setup for each test case
  LagrangeP1Line3DFixture() : mapped_coords((ETYPE::MappedCoordsT() << .2).finished()), nodes((NodesT() << 5., 7., 2.,
                                                                              10., 3., 3.).finished())
  {
  }

  /// common tear-down for each test case
  ~LagrangeP1Line3DFixture()
  {
  }

  /// Fills the given coordinate and connectivity data to create a helix along the Z-axis, consisting of ETYPE elements
  void create_helix(Table<Real>& coordinates, Table<Uint>& connectivity, const Real radius, const Real height, const Real tours, const Uint segments)
  {
    const Uint dim = ETYPE::dimension;
    const Uint nb_nodes = ETYPE::nb_nodes;
    const Real start_angle = 0.;
    const Real end_angle = tours*2.*Consts::pi();

    coordinates.set_row_size(dim);
    Table<Real>::ArrayT& coord_array = coordinates.array();
    coord_array.resize(boost::extents[segments + 1][dim]);

    connectivity.set_row_size(nb_nodes);
    Table<Uint>::ArrayT& conn_array = connectivity.array();
    conn_array.resize(boost::extents[segments][nb_nodes]);
    const Real height_step = height / segments;
    for(Uint u = 0; u != segments; ++u)
    {
      const Real theta = start_angle + (end_angle - start_angle) * (static_cast<Real>(u) / static_cast<Real>(segments));
      Table<Real>::Row coord_row = coord_array[u];

      coord_row[XX] = radius * cos(theta);
      coord_row[YY] = radius * sin(theta);
      coord_row[ZZ] = u*height_step;

      Table<Uint>::Row nodes = conn_array[u];
      nodes[0] = u;
      nodes[1] = u+1;
    }
    Table<Real>::Row coord_row = coord_array[segments];
    coord_row[XX] = radius * cos(end_angle);
    coord_row[YY] = radius * sin(end_angle);
    coord_row[ZZ] = segments * height_step;
  }

  const ETYPE::MappedCoordsT mapped_coords;
  const NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      ETYPE::JacobianT jac;
      ETYPE::compute_jacobian(mapped_coords, m_nodes, jac);
      return jac.norm();
    }
    ETYPE::MappedCoordsT mapped_coords;
  private:
    const NodesT& m_nodes;
  };

  /// Returns the norm of the tangen vector to the curve
  struct TangentVectorNorm {

    template<typename NodesT>
    Real operator()(const ETYPE::MappedCoordsT& mapped_coords, const NodesT& nodes)
    {
      ETYPE::JacobianT jac;
      ETYPE::compute_jacobian(mapped_coords, nodes, jac);
      Real result = 0.;
      for(Uint i = 0; i != ETYPE::dimension; ++i)
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
void integrate_region(ResultT& result, FunctorT functor, const Table<Real>& coordinates, const Table<Uint>& connectivity)
{
  const Uint nb_elems = connectivity.array().size();
  for(Uint elem_idx = 0; elem_idx != nb_elems; ++ elem_idx)
  {
    ETYPE::NodesT nodes;
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
  static const ETYPE::MappedCoordsT mapped_coords((ETYPE::MappedCoordsT() << mu).finished());
  result += w * functor(mapped_coords, nodes);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LagrangeP1Line3DSuite, LagrangeP1Line3DFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Length )
{
  NodesT nodes_line3D;
  nodes_line3D << 2., 2., 2.,
                  1., 1., 1.;
  BOOST_CHECK_EQUAL(ETYPE::length(nodes_line3D),std::sqrt(3.));
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  ETYPE::SF::ValueT reference_result;
  reference_result << 0.4, 0.6;
  ETYPE::SF::ValueT result;
  ETYPE::SF::compute_value(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  ETYPE::SF::GradientT result;
  ETYPE::SF::GradientT expected(-0.5, 0.5);
  ETYPE::SF::compute_gradient(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  ETYPE::JacobianT expected(2.5, -2., 0.5);
  ETYPE::JacobianT result;
  ETYPE::compute_jacobian(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  ConstFunctor ftor(nodes);
  const Real len = ETYPE::length(nodes);

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
  boost::shared_ptr< Table<Real> > coordinates(common::allocate_component< Table<Real> >(mesh::Tags::coordinates()));
  boost::shared_ptr< Table<Uint> > connectivity(common::allocate_component< Table<Uint> >("connectivity"));
  create_helix(*coordinates, *connectivity, radius, height, tours, segments);

  // Check the length, using the line integral of one times the norm of the tangent vector
  Real length = 0.;
  integrate_region(length, TangentVectorNorm(), *coordinates, *connectivity);
  BOOST_CHECK_CLOSE(length, tours*sqrt((square(2.*Consts::pi()*radius)+square(height/tours))), 0.01);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

