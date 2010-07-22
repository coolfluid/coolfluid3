#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Line3DLagrangeP1 shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Math/MathConsts.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementNodes.hpp"
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

struct LagrangeSFLine3DLagrangeP1Fixture
{
  typedef std::vector<RealVector> NodesT;
  /// common setup for each test case
  LagrangeSFLine3DLagrangeP1Fixture() : mapped_coords(init_mapped_coords()), nodes(init_nodes())
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
    const Real end_angle = tours*2.*MathConsts::RealPi();

    coordinates.initialize(dim);
    CArray::Array& coord_array = coordinates.array();
    coord_array.resize(boost::extents[segments + 1][dim]);

    connectivity.initialize(nb_nodes);
    CTable::ConnectivityTable& conn_array = connectivity.table();
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
      return sqrt(jac[XX]*jac[XX] + jac[YY]*jac[YY] + jac[ZZ]*jac[ZZ]);
    }
  private:
    const NodesT& m_nodes;
  };

  /// Returns the norm of the tangen vector to the curve
  struct TangentVectorNorm {

    template<typename NodesT, typename GeoSF>
    Real operator()(const RealVector& mapped_coords, const NodesT& nodes, const GeoSF& sf)
    {
      cf_assert(GeoSF::dimensionality == 1);
      RealMatrix jac(GeoSF::dimensionality, GeoSF::dimension);
      GeoSF::jacobian(mapped_coords, nodes, jac);
      Real result = 0.;
      for(Uint i = 0; i != GeoSF::dimension; ++i)
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

private:
  /// Workaround for boost:assign ambiguity
  RealVector init_mapped_coords()
  {
    return list_of(0.2);
  }

  /// Workaround for boost:assign ambiguity
  NodesT init_nodes()
  {
    const RealVector c0 = list_of(5.)(7.)(2.);
    const RealVector c1 = list_of(10.)(3.)(3.);
    return list_of(c0)(c1);
  }
};

/// Mimic a possible new integration interface
template<typename ResultT, typename FunctorT, typename GeoSF>
void integrate_region(ResultT& result, FunctorT functor, const CArray& coordinates, const CTable& connectivity, const GeoSF& geo_sf)
{
  const Uint nb_elems = connectivity.table().size();
  for(Uint elem_idx = 0; elem_idx != nb_elems; ++ elem_idx)
  {
    const ConstElementNodeView nodes(coordinates, connectivity.table()[elem_idx]);
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

BOOST_FIXTURE_TEST_SUITE( Line3DLagrangeP1Suite, LagrangeSFLine3DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Length )
{
  boost::multi_array<Real,2> nodes_line3D (boost::extents[2][3]);
  nodes_line3D[0][XX] = 2.0;     nodes_line3D[0][YY] = 2.0;     nodes_line3D[0][ZZ] = 2.0;
  nodes_line3D[1][XX] = 1.0;     nodes_line3D[1][YY] = 1.0;     nodes_line3D[1][ZZ] = 1.0;
  BOOST_CHECK_EQUAL(Line3DLagrangeP1::length(nodes_line3D),std::sqrt(3.));
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const RealVector reference_result = list_of(0.4)(0.6)(1.);
  RealVector result(Line3DLagrangeP1::nb_nodes);
  Line3DLagrangeP1::shape_function(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  RealMatrix result(Line3DLagrangeP1::nb_nodes, Line3DLagrangeP1::dimension);
  RealMatrix expected(Line3DLagrangeP1::nb_nodes, Line3DLagrangeP1::dimension);
  expected(0,0) = -0.5;
  expected(1,0) = 0.5;
  Line3DLagrangeP1::mapped_gradient(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  RealMatrix expected(Line3DLagrangeP1::dimensionality, Line3DLagrangeP1::dimension);
  expected(KSI,XX) = 2.5;
  expected(KSI,YY) = -2.;
  expected(KSI,ZZ) = 0.5;
  RealMatrix result(Line3DLagrangeP1::dimensionality, Line3DLagrangeP1::dimension);
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

  Gauss<Line3DLagrangeP1>::integrateElement(ftor, result1);
  Gauss<Line3DLagrangeP1, Line3DLagrangeP1, 2>::integrateElement(ftor, result2);
  Gauss<Line3DLagrangeP1, Line3DLagrangeP1, 4>::integrateElement(ftor, result4);
  Gauss<Line3DLagrangeP1, Line3DLagrangeP1, 8>::integrateElement(ftor, result8);
  Gauss<Line3DLagrangeP1, Line3DLagrangeP1, 16>::integrateElement(ftor, result16);
  Gauss<Line3DLagrangeP1, Line3DLagrangeP1, 32>::integrateElement(ftor, result32);

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

  Line3DLagrangeP1 aSF; /* temporary solution, on Mac it tries to use copy-constructor */

  // Check the length, using the line integral of one times the norm of the tangent vector
  Real length = 0.;
  integrate_region(length, TangentVectorNorm(), coordinates, connectivity, aSF);
  BOOST_CHECK_CLOSE(length, tours*sqrt((square(2.*MathConsts::RealPi()*radius)+square(height/tours))), 0.01);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

