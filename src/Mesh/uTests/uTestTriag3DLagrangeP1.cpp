#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Triag3DLagrangeP1 shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Math/MathConsts.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/ElementNodes.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Triag3DLagrangeP1.hpp"



#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Math;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::SF;
using namespace CF::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

struct Triag3DLagrangeP1Fixture
{
  typedef std::vector<RealVector> NodesT;
  /// common setup for each test case
  Triag3DLagrangeP1Fixture() : mapped_coords(init_mapped_coords()), nodes(init_nodes())
  {
  }

  /// common tear-down for each test case
  ~Triag3DLagrangeP1Fixture()
  {
  }

  /// Fills the given coordinate and connectivity data to create a cylinder along the Z-axis, consisting of Triag3DLagrangeP1 elements
  void create_cylinder(CArray& coordinates, CTable& connectivity, const Real radius, const Uint u_segments, const Uint v_segments, const Real height, const Real start_angle = 0., const Real end_angle = 2.*MathConsts::RealPi())
  {
    const Uint dim = Triag3DLagrangeP1::dimension;
    const Uint nb_nodes = Triag3DLagrangeP1::nb_nodes;
    const bool closed = std::abs(std::abs(end_angle - start_angle) - 2.0*MathConsts::RealPi()) < MathConsts::RealEps();

    coordinates.initialize(dim);
    CArray::Array& coord_array = coordinates.array();
    coord_array.resize(boost::extents[(u_segments + (!closed)) * (v_segments+1)][dim]);

    connectivity.initialize(nb_nodes);
    CTable::ConnectivityTable& conn_array = connectivity.table();
    conn_array.resize(boost::extents[2 * u_segments * v_segments][nb_nodes]);
    const Real v_step = height / v_segments;

    if(!closed)
    {
      for(Uint v = 0; v <= v_segments; ++v)
      {
        const Real z_coord = v_step * static_cast<Real>(v);
        for(Uint u = 0; u <= u_segments; ++u)
        {
          const Real theta = start_angle + (end_angle - start_angle) * (static_cast<Real>(u) / static_cast<Real>(u_segments));
          CArray::Row coord_row = coord_array[v*(u_segments+1) + u];

          coord_row[XX] = radius * cos(theta);
          coord_row[YY] = radius * sin(theta);
          coord_row[ZZ] = z_coord;
        }
      }

      for(Uint v = 0; v != v_segments; ++v)
      {
        //const Real z_coord = v_step * static_cast<Real>(v);
        for(Uint u = 0; u != u_segments; ++u)
        {
          const Uint node0 = v*(u_segments+1) + u;
          const Uint node1 = node0 + 1;
          const Uint node3 = (v+1)*(u_segments+1) + u;
          const Uint node2 = node3 + 1;

          CTable::Row nodes1 = conn_array[2*(v*u_segments + u)];
          CTable::Row nodes2 = conn_array[2*(v*u_segments + u) + 1];

          nodes1[0] = node0;
          nodes1[1] = node1;
          nodes1[2] = node2;

          nodes2[0] = node2;
          nodes2[1] = node3;
          nodes2[2] = node0;
        }
      }
    }
    else // closed loop
    {
      for(Uint v = 0; v <= v_segments; ++v)
      {
        const Real z_coord = v_step * static_cast<Real>(v);
        for(Uint u = 0; u != u_segments; ++u)
        {
          const Real theta = start_angle + (end_angle - start_angle) * (static_cast<Real>(u) / static_cast<Real>(u_segments));
          CArray::Row coord_row = coord_array[v*u_segments + u];

          coord_row[XX] = radius * cos(theta);
          coord_row[YY] = radius * sin(theta);
          coord_row[ZZ] = z_coord;
        }
      }

      for(Uint v = 0; v != v_segments; ++v)
      {
        //const Real z_coord = v_step * static_cast<Real>(v);
        for(Uint u = 0; u != u_segments; ++u)
        {
          const Uint node0 = v*u_segments + u;
          const Uint node1 = node0 + 1;
          const Uint node3 = (v+1)*u_segments + u;
          const Uint node2 = node3 + 1;

          CTable::Row nodes1 = conn_array[2*(v*u_segments + u)];
          CTable::Row nodes2 = conn_array[2*(v*u_segments + u) + 1];

          nodes1[0] = node0;
          nodes1[1] = node1;
          nodes1[2] = node2;

          nodes2[0] = node2;
          nodes2[1] = node3;
          nodes2[2] = node0;
        }
        conn_array[2 * (v*u_segments + u_segments-1)][1] = v*u_segments;
        conn_array[2 * (v*u_segments + u_segments-1)][2] = (v+1)*u_segments;
        conn_array[2 * (v*u_segments + u_segments-1) + 1][0] = (v+1)*u_segments;
      }
    }
  }

  const RealVector mapped_coords;
  const NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : m_nodes(node_list) {}
    template<typename GeoSF, typename SolSF>
    Real valTimesDetJacobian(const RealVector& mappedCoords)
    {
      RealVector result(GeoSF::dimension);
      GeoSF::normal(mappedCoords, m_nodes, result);
      return result.norm2();
    }
  private:
    const NodesT& m_nodes;
  };

  /// Returns the norm of the normal vector to the curve or surface element (equal to tangent in the case of Line2D)
  struct NormalVectorNorm {

    template<typename NodesT, typename GeoSF>
    Real operator()(const RealVector& mapped_coords, const NodesT& nodes, const GeoSF& sf)
    {
      RealVector result(GeoSF::dimension);
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
      RealVector normal(GeoSF::dimension);
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
      RealVector nodal_p(GeoSF::nb_nodes);
      for(Uint i = 0; i != GeoSF::nb_nodes; ++i)
        nodal_p[i] = pressure(theta(nodes[i]));

      // The local normal
      RealVector normal(GeoSF::dimension);
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
    static const Real m_rho = 1.225;

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
    return list_of(0.1)(0.8);
  }

  /// Workaround for boost:assign ambiguity
  NodesT init_nodes()
  {
    const CF::RealVector c0 = list_of(0.5)(0.3)(0.);
    const CF::RealVector c1 = list_of(1.1)(1.2)(0.5);
    const CF::RealVector c2 = list_of(0.8)(2.1)(1.);
    return list_of(c0)(c1)(c2);
  }
};

/// Mimic a possible new integration interface
template<typename ResultT, typename FunctorT, typename GeoSF>
void integrate_region(ResultT& result, FunctorT functor, const CArray& coordinates, const CTable& connectivity, const GeoSF& geo_sf)
{
  const Uint nb_elems = connectivity.table().size();
  for(Uint elem_idx = 0; elem_idx != nb_elems; ++ elem_idx)
  {
    //const CTable::ConstRow& r = connectivity.table()[elem_idx];
    const ConstElementNodeView nodes(coordinates, connectivity.table()[elem_idx]);
    integrate_element(result, functor, nodes, geo_sf);
  }
}

/// Mimic a possible new integration interface. Integration over a single element
template<typename ResultT, typename FunctorT, typename NodesT, typename GeoSF>
void integrate_element(ResultT& result, FunctorT functor, const NodesT& nodes, const GeoSF& sf)
{
  static const double mu = 0.3333333333333333333333333;
  static const double w = 0.5;
  static const RealVector mapped_coords = boost::assign::list_of(mu)(mu);
  result += w * functor(mapped_coords, nodes, sf);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Triag3DLagrangeP1Suite, Triag3DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Area )
{
  boost::multi_array<Real,2> nodes_triag3D (boost::extents[3][3]);
  nodes_triag3D[0][XX] = 0.0;     nodes_triag3D[0][YY] = 0.0;     nodes_triag3D[0][ZZ] = 0.0;
  nodes_triag3D[1][XX] = 1.0;     nodes_triag3D[1][YY] = 0.0;     nodes_triag3D[1][ZZ] = 1.0;
  nodes_triag3D[2][XX] = 1.0;     nodes_triag3D[2][YY] = 1.0;     nodes_triag3D[2][ZZ] = 1.0;
  BOOST_CHECK_EQUAL(Triag3DLagrangeP1::area(nodes_triag3D), std::sqrt(2.)/2.);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const CF::RealVector reference_result = list_of(0.1)(0.1)(0.8);
  CF::RealVector result(3);
  Triag2DLagrangeP1::shape_function(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  CF::RealMatrix expected(3, 2);
  expected(0,0) = -1.;
  expected(0,1) = -1.;
  expected(1,0) = 1.;
  expected(1,1) = 0.;
  expected(2,0) = 0.;
  expected(2,1) = 1.;
  CF::RealMatrix result(3, 2);
  Triag2DLagrangeP1::mapped_gradient(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  CF::RealMatrix expected(Triag3DLagrangeP1::dimensionality, Triag3DLagrangeP1::dimension);
  expected(KSI,XX) = 0.6;
  expected(KSI,YY) = 0.9;
  expected(KSI,ZZ) = 0.5;

  expected(ETA,XX) = 0.3;
  expected(ETA,YY) = 1.8;
  expected(ETA,ZZ) = 1.;

  CF::RealMatrix result(Triag3DLagrangeP1::dimensionality, Triag3DLagrangeP1::dimension);
  Triag3DLagrangeP1::jacobian(mapped_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 15);
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  ConstFunctor ftor(nodes);
  const Real area = Triag3DLagrangeP1::area(nodes);

  Real result = 0.0;

  Gauss<Triag3DLagrangeP1>::integrateElement(ftor, result);

  BOOST_CHECK_CLOSE(result, area, 0.001);
}

/// Surface integrals can be done in 2D by making use of the normal vector to a curve.
/// The components of this vector are the elements of the jacobian. For scalar functions, we
/// need to integrate the function multiplied with the norm of the normal vector, for vector
/// fields we integrate the scalar product of the function and the normal vector
BOOST_AUTO_TEST_CASE( SurfaceIntegral )
{
  // Create an approximation of a circle
  const Real radius = 1.;
  const Uint u_segments = 100;
  const Uint v_segments = 24;
  const Real height = 3.;

  // complete circle
  CArray coordinates("coordinates");
  CTable connectivity("connectivity");
  create_cylinder(coordinates, connectivity, radius, u_segments, v_segments, height);

  Triag3DLagrangeP1 aSF; /* temporary solution, on Mac it tries to use copy-constructor */

  // Check the area
  Real area = 0.;
  integrate_region(area, NormalVectorNorm(), coordinates, connectivity, aSF);
  BOOST_CHECK_CLOSE(area, 2.*MathConsts::RealPi()*radius*height, 0.1);

  // Flux from a constant vector field through a closed surface should be 0
  Real zero_flux = 0.;
  const RealVector field_vector = boost::assign::list_of(0.35)(1.25)(0.);
  integrate_region(zero_flux, ConstVectorField(field_vector), coordinates, connectivity, aSF);
  BOOST_CHECK_SMALL(zero_flux, 1e-14);
}

BOOST_AUTO_TEST_CASE( ArcIntegral )
{
  // half cylinder arc
  CArray arc_coordinates("coordinates");
  CTable arc_connectivity("connectivity");
  create_cylinder(arc_coordinates, arc_connectivity, 1., 100, 24, 3., 0., MathConsts::RealPi());
  Real arc_flux = 0.;
  const RealVector y_vector = boost::assign::list_of(0.)(1.)(0.);
  Triag3DLagrangeP1 aSF; /* temporary solution, on Mac it tries to use copy-constructor */
  integrate_region(arc_flux, ConstVectorField(y_vector), arc_coordinates, arc_connectivity, aSF);
  BOOST_CHECK_CLOSE(arc_flux, 6., 0.01);
}

/// Lift produced by a rotating cylinder
BOOST_AUTO_TEST_CASE( RotatingCylinder )
{
  // Create an approximation of a circle
  const Real radius = 1.;
  const Uint u_segments = 1000;
  const Uint v_segments = 100;
  const Real height = 3.;

  // complete cylinder
  CArray coordinates("coordinates");
  CTable connectivity("connectivity");
  create_cylinder(coordinates, connectivity, radius, u_segments, v_segments, height);

  // Rotating cylinder in uniform flow
  const Real u = 300.;
  const Real circulation = 975.;
  RealVector force(0.,3);
  Triag3DLagrangeP1 aSF; /* temporary solution, on Mac it tries to use copy-constructor */
  integrate_region(force, RotatingCylinderPressure(radius, circulation, u), coordinates, connectivity, aSF);
  BOOST_CHECK_CLOSE(force[YY], height * 1.225*u*circulation, 0.001); // lift according to theory
  BOOST_CHECK_SMALL(force[XX], 1e-8); // Drag should be zero
  BOOST_CHECK_SMALL(force[ZZ], 1e-8);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

