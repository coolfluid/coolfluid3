// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Line2DLagrangeP1 shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Math/MathConsts.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementNodes.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Line2DLagrangeP1.hpp"



#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Math;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::SF;
using namespace CF::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

struct LagrangeSFLine2DLagrangeP1Fixture
{
  typedef std::vector<RealVector> NodesT;
  /// common setup for each test case
  LagrangeSFLine2DLagrangeP1Fixture() : mapped_coords(init_mapped_coords()), nodes(init_nodes())
  {
  }

  /// common tear-down for each test case
  ~LagrangeSFLine2DLagrangeP1Fixture()
  {
  }

  /// Fills the given coordinate and connectivity data to create a 2D circle, consisting of Line2DLagrangeP1 elements
  void create_circle_2d(CArray& coordinates, CTable& connectivity, const Real radius, const Uint segments, const Real start_angle = 0., const Real end_angle = 2.*MathConsts::RealPi())
  {
    const Uint dim = Line2DLagrangeP1::dimension;
    const Uint nb_nodes = Line2DLagrangeP1::nb_nodes;
    const bool closed = std::abs(std::abs(end_angle - start_angle) - 2.0*MathConsts::RealPi()) < MathConsts::RealEps();

    coordinates.initialize(dim);
    CArray::ArrayT& coord_array = coordinates.array();
    coord_array.resize(boost::extents[segments + (!closed)][dim]);

    connectivity.initialize(nb_nodes);
    CTable::ArrayT& conn_array = connectivity.array();
    conn_array.resize(boost::extents[segments][nb_nodes]);
    for(Uint u = 0; u != segments; ++u)
    {
      const Real theta = start_angle + (end_angle - start_angle) * (static_cast<Real>(u) / static_cast<Real>(segments));
      CArray::Row coord_row = coord_array[u];

      coord_row[XX] = radius * cos(theta);
      coord_row[YY] = radius * sin(theta);

      CTable::Row nodes = conn_array[u];
      nodes[0] = u;
      nodes[1] = u+1;
    }
    if(closed)
    {
      conn_array[segments-1][1] = 0;
    }
    else
    {
      CArray::Row coord_row = coord_array[segments];
      coord_row[XX] = radius * cos(end_angle);
      coord_row[YY] = radius * sin(end_angle);
    }
  }

  const RealVector mapped_coords;
  const NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : m_nodes(node_list) {}
    template<typename GeoShapeF, typename SolShapeF>
    Real valTimesDetJacobian(const RealVector& mappedCoords)
    {
      RealMatrix jac(GeoShapeF::dimensionality, GeoShapeF::dimension);
      GeoShapeF::jacobian(mappedCoords, m_nodes, jac);
      return sqrt(jac[0]*jac[0] + jac[1]*jac[1]);
    }
  private:
    const NodesT& m_nodes;
  };

  /// Returns the norm of the normal vector to the curve or surface element (equal to tangent in the case of Line2D)
  struct NormalVectorNorm {

    template<typename NodesT, typename GeoSF>
    Real operator()(const RealVector& mapped_coords, const NodesT& nodes, const GeoSF& sf)
    {
      RealVector result(2);
      GeoSF::normal(mapped_coords, nodes, result);
      return result.norm2();
    }

  };

  /// Returns the scalar product of a constant vector field and the local element normal
  struct ConstVectorField {

    ConstVectorField(const RealVector& vector) : m_vector(vector) {}

    template<typename NodesT, typename GeoSF>
    Real operator()(const RealVector& mapped_coords, const NodesT& nodes, const GeoSF& sf)
    {
      RealVector normal(2);
      GeoSF::normal(mapped_coords, nodes, normal);
      return MathFunctions::innerProd(normal, m_vector);
    }

  private:
    const RealVector m_vector;

  };

  /// Returns the static pressure around a rotating cylinder in a horizontal,
  /// uniform velocity field U, multiplied with the local normal vector
  struct RotatingCylinderPressure {

    RotatingCylinderPressure(const Real radius, const Real circulation, const Real U) :
      m_radius(radius), m_circulation(circulation), m_u(U) {}

    template<typename NodesT, typename GeoSF>
    RealVector operator()(const RealVector& mapped_coords, const NodesT& nodes, const GeoSF& sf)
    {
      // The pressures to interpolate
      RealVector nodal_p(2);
      nodal_p[0] = pressure(theta(nodes[0]));
      nodal_p[1] = pressure(theta(nodes[1]));

      // The local normal
      RealVector normal(2);
      GeoSF::normal(mapped_coords, nodes, normal);

      // Interpolate the pressure
      Real p;
      sf(mapped_coords, nodal_p, p);

      return normal * p;
    }

  private:
    const Real m_radius;
    const Real m_circulation;
    const Real m_u;
    static const Real m_rho;

    // Reconstruct the value of theta, based on the coordinates
    Real theta(const RealVector& coords)
    {
      return atan2(coords[YY], coords[XX]);
    }

    // Pressure in function of theta
    Real pressure(const Real theta)
    {
      Real tmp = (2. * m_u * sin(theta) + m_circulation / (2. * MathConsts::RealPi() * m_radius));
      return 0.5 * m_rho * tmp * tmp;
    }

  };

private:
  /// Workaround for boost:assign ambiguity
  RealVector init_mapped_coords()
  {
    return list_of(0.2);
  }

  /// Workaround for boost:assign ambiguity
  NodesT init_nodes()
  {
    const RealVector c0 = list_of(5.)(7.);
    const RealVector c1 = list_of(10.)(3.);
    return list_of(c0)(c1);
  }
};

const Real LagrangeSFLine2DLagrangeP1Fixture::RotatingCylinderPressure::m_rho = 1.225;

/// Mimic a possible new integration interface
template<typename ResultT, typename FunctorT, typename GeoSF>
void integrate_region(ResultT& result, FunctorT functor, const CArray& coordinates, const CTable& connectivity, const GeoSF& geo_sf)
{
  const Uint nb_elems = connectivity.array().size();
  for(Uint elem_idx = 0; elem_idx != nb_elems; ++ elem_idx)
  {
    const ConstElementNodeView nodes(coordinates, connectivity.array()[elem_idx]);
    integrate_element(result, functor, nodes, geo_sf);
  }
}

/// Mimic a possible new integration interface. Integration over a single element
template<typename ResultT, typename FunctorT, typename NodesT, typename GeoSF>
void integrate_element(ResultT& result, FunctorT functor, const NodesT& nodes, const GeoSF& sf)
{
  static const double mu = 0.;
  static const double w = 2.;
  static const RealVector mapped_coords = boost::assign::list_of(mu);
  result += w * functor(mapped_coords, nodes, sf);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Line2DLagrangeP1Suite, LagrangeSFLine2DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Area )
{
  ElementType::NodesT nodes_line2D(2, 2);
  nodes_line2D[0][XX] = 2.0;     nodes_line2D[0][YY] = 2.0;
  nodes_line2D[1][XX] = 1.0;     nodes_line2D[1][YY] = 1.0;
  BOOST_CHECK_EQUAL(Line2DLagrangeP1::area(nodes_line2D),std::sqrt(2.));
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const RealVector reference_result = list_of(0.4)(0.6);
  RealVector result(Line2DLagrangeP1::nb_nodes);
  Line2DLagrangeP1::shape_function(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  RealMatrix result(Line2DLagrangeP1::dimension, Line2DLagrangeP1::nb_nodes);
  RealMatrix expected(Line2DLagrangeP1::dimension, Line2DLagrangeP1::nb_nodes);
  expected(0,0) = -0.5;
  expected(0,1) = 0.5;
  Line2DLagrangeP1::mapped_gradient(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  RealMatrix expected(Line2DLagrangeP1::dimensionality, Line2DLagrangeP1::dimension);
  expected(KSI,XX) = 2.5;
  expected(KSI,YY) = -2.;
  RealMatrix result(Line2DLagrangeP1::dimensionality, Line2DLagrangeP1::dimension);
  Line2DLagrangeP1::jacobian(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  ConstFunctor ftor(nodes);
  const Real vol = Line2DLagrangeP1::area(nodes);

  Real result1 = 0.0;
  Real result2 = 0.0;
  Real result4 = 0.0;
  Real result8 = 0.0;
  Real result16 = 0.0;
  Real result32 = 0.0;

  Gauss<Line2DLagrangeP1>::integrateElement(ftor, result1);
  Gauss<Line2DLagrangeP1, Line2DLagrangeP1, 2>::integrateElement(ftor, result2);
  Gauss<Line2DLagrangeP1, Line2DLagrangeP1, 4>::integrateElement(ftor, result4);
  Gauss<Line2DLagrangeP1, Line2DLagrangeP1, 8>::integrateElement(ftor, result8);
  Gauss<Line2DLagrangeP1, Line2DLagrangeP1, 16>::integrateElement(ftor, result16);
  Gauss<Line2DLagrangeP1, Line2DLagrangeP1, 32>::integrateElement(ftor, result32);

  BOOST_CHECK_LT(boost::accumulators::max(test(result1, vol).ulps), 1);
  BOOST_CHECK_LT(boost::accumulators::max(test(result2, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result4, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result8, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result16, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result32, vol).ulps), 5);
}

/// Surface integrals can be done in 2D by making use of the normal vector to a curve.
/// The components of this vector are the elements of the jacobian. For scalar functions, we
/// need to integrate the function multiplied with the norm of the normal vector, for vector
/// fields we integrate the scalar product of the function and the normal vector
BOOST_AUTO_TEST_CASE( SurfaceIntegral )
{
  // Create an approximation of a circle
  //const Real radius = 1.;
  //const Uint segments = 100;

  // complete circle
  CArray coordinates("coordinates");
  CTable connectivity("connectivity");
  create_circle_2d(coordinates, connectivity, 1., 100);

  Line2DLagrangeP1 aSF; /* temporary solution, on Mac it tries to use copy-constructor */

  // Check the length, using the line integral of one times the norm of the tangent vector
  Real length = 0.;
  integrate_region(length, NormalVectorNorm(), coordinates, connectivity, aSF);
  BOOST_CHECK_CLOSE(length, 2.*MathConsts::RealPi(), 0.1);

  // Flux from a constant vector field through a closed surface should be 0
  Real zero_flux = 0.;
  const RealVector field_vector = boost::assign::list_of(0.35)(1.25);
  integrate_region(zero_flux, ConstVectorField(field_vector), coordinates, connectivity, aSF);
  BOOST_CHECK_SMALL(zero_flux, 1e-14);
}

BOOST_AUTO_TEST_CASE( ArcIntegral )
{
  // half circle arc, so the flux of a uniform field of unit vectors should equal the diameter
  CArray arc_coordinates("coordinates");
  CTable arc_connectivity("connectivity");
  create_circle_2d(arc_coordinates, arc_connectivity, 1., 100, 0., MathConsts::RealPi());
  Real arc_flux = 0.;
  const RealVector y_vector = boost::assign::list_of(0.)(1.);
  Line2DLagrangeP1 aSF; /* temporary solution, on Mac it tries to use copy-constructor */
  integrate_region(arc_flux, ConstVectorField(y_vector), arc_coordinates, arc_connectivity, aSF);
  BOOST_CHECK_CLOSE(arc_flux, 2., 0.01);
}

/// Lift produced by a rotating cylinder
BOOST_AUTO_TEST_CASE( RotatingCylinder )
{
  // Create an approximation of a circle
  const Real radius = 1.;
  const Uint segments = 10000;

  // complete circle
  CArray coordinates("coordinates");
  CTable connectivity("connectivity");
  create_circle_2d(coordinates, connectivity, 1., segments);

  // Rotating cylinder in uniform flow
  const Real u = 300.;
  const Real circulation = 975.;
  RealVector force(0.,2);
  Line2DLagrangeP1 aSF; /* temporary solution, on Mac it tries to use copy-constructor */
  integrate_region(force, RotatingCylinderPressure(radius, circulation, u), coordinates, connectivity, aSF);
  BOOST_CHECK_CLOSE(force[YY], 1.225*u*circulation, 0.001); // lift according to theory
  BOOST_CHECK_SMALL(force[XX], 1e-8); // Drag should be zero
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

