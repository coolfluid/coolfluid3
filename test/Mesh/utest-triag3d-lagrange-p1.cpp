// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Triag3DLagrangeP1 shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Math/MathConsts.hpp"

#include "Mesh/CTable.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Triag3DLagrangeP1.hpp"
#include "Mesh/ElementData.hpp"


#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Math;
using namespace CF::Math::MathConsts;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::SF;
using namespace CF::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Triag3DLagrangeP1 SFT;

struct Triag3DLagrangeP1Fixture
{
  typedef SFT::NodeMatrixT NodesT;
  /// common setup for each test case
  Triag3DLagrangeP1Fixture() : mapped_coords(0.1, 0.8), nodes((NodesT() << 0.5, 0.3, 0.,
                                                                           1.1, 1.2, 0.5,
                                                                           0.8, 2.1, 1.).finished())
  {
  }

  /// common tear-down for each test case
  ~Triag3DLagrangeP1Fixture()
  {
  }

  /// Fills the given coordinate and connectivity data to create a cylinder along the Z-axis, consisting of Triag3DLagrangeP1 elements
  void create_cylinder(CTable<Real>& coordinates, CTable<Uint>& connectivity, const Real radius, const Uint u_segments, const Uint v_segments, const Real height, const Real start_angle = 0., const Real end_angle = 2.*MathConsts::pi())
  {
    const Uint dim = Triag3DLagrangeP1::dimension;
    const Uint nb_nodes = Triag3DLagrangeP1::nb_nodes;
    const bool closed = std::abs(std::abs(end_angle - start_angle) - 2.0*MathConsts::pi()) < eps();

    coordinates.set_row_size(dim);
    CTable<Real>::ArrayT& coord_array = coordinates.array();
    coord_array.resize(boost::extents[(u_segments + (!closed)) * (v_segments+1)][dim]);

    connectivity.set_row_size(nb_nodes);
    CTable<Uint>::ArrayT& conn_array = connectivity.array();
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
          CTable<Real>::Row coord_row = coord_array[v*(u_segments+1) + u];

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

          CTable<Uint>::Row nodes1 = conn_array[2*(v*u_segments + u)];
          CTable<Uint>::Row nodes2 = conn_array[2*(v*u_segments + u) + 1];

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
          CTable<Real>::Row coord_row = coord_array[v*u_segments + u];

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

          CTable<Uint>::Row nodes1 = conn_array[2*(v*u_segments + u)];
          CTable<Uint>::Row nodes2 = conn_array[2*(v*u_segments + u) + 1];

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

  const SFT::MappedCoordsT mapped_coords;
  const NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      SFT::CoordsT result;
      SFT::normal(mapped_coords, m_nodes, result);
      return result.norm();
    }
    SFT::MappedCoordsT mapped_coords;
  private:
    const NodesT& m_nodes;
  };

  /// Returns the norm of the normal vector to the curve or surface element (equal to tangent in the case of Line2D)
  struct NormalVectorNorm {

    Real operator()(const RealVector& mapped_coords, const NodesT& nodes)
    {
      SFT::CoordsT result;
      SFT::normal(mapped_coords, nodes, result);
      return result.norm();
    }

  };

  /// Returns the scalar product of a constant vector field and the local element normal
  struct ConstVectorField {

    ConstVectorField(const RealVector& vector) : m_vector(vector) {}

    Real operator()(const RealVector& mapped_coords, const NodesT& nodes)
    {
      SFT::CoordsT normal;
      SFT::normal(mapped_coords, nodes, normal);
      return MathFunctions::inner_product(normal, m_vector);
    }

  private:
    const SFT::CoordsT m_vector;

  };

  /// Returns the static pressure around a rotating cylinder in a horizontal,
  /// uniform velocity field U, multiplied with the local normal vector
  struct RotatingCylinderPressure
  {

    RotatingCylinderPressure(const Real radius, const Real circulation, const Real U) :
      m_radius(radius), m_circulation(circulation), m_u(U) {}

    SFT::CoordsT operator()(const RealVector& mapped_coords, const NodesT& nodes)
    {
      // The pressures to interpolate
      RealVector3 nodal_p;
      nodal_p[0] = pressure(theta(nodes.row(0)));
      nodal_p[1] = pressure(theta(nodes.row(1)));
      nodal_p[2] = pressure(theta(nodes.row(2)));

      // The local normal
      SFT::CoordsT normal;
      SFT::normal(mapped_coords, nodes, normal);

      // Interpolate the pressure
      SFT::ShapeFunctionsT sf_mat;
      SFT::shape_function_value(mapped_coords, sf_mat);

      return normal * (sf_mat * nodal_p);
    }

  private:
    const Real m_radius;
    const Real m_circulation;
    const Real m_u;
    static const Real m_rho;

    // Reconstruct the value of theta, based on the coordinates
    Real theta(const SFT::CoordsT& coords)
    {
      return atan2(coords[YY], coords[XX]);
    }

    // Pressure in function of theta
    Real pressure(const Real theta)
    {
      Real tmp = (2. * m_u * sin(theta) + m_circulation / (2. * MathConsts::pi() * m_radius));
      return 0.5 * m_rho * tmp * tmp;
    }
  };
};

const Real Triag3DLagrangeP1Fixture::RotatingCylinderPressure::m_rho = 1.225;

template<typename ResultT, typename FunctorT>
void integrate_region(ResultT& result, FunctorT functor, const CTable<Real>& coordinates, const CTable<Uint>& connectivity)
{
  const Uint nb_elems = connectivity.array().size();
  for(Uint elem_idx = 0; elem_idx != nb_elems; ++ elem_idx)
  {
    //const CTable<Uint>::ConstRow& r = connectivity.array()[elem_idx];
    typename SFT::NodeMatrixT nodes;
    fill(nodes, coordinates, connectivity.array()[elem_idx]);
    integrate_element(result, functor, nodes);
  }
}

template<typename ResultT, typename FunctorT, typename NodesT>
void integrate_element(ResultT& result, FunctorT functor, const NodesT& nodes)
{
  static const double mu = 0.3333333333333333333333333;
  static const double w = 0.5;
  static const SFT::MappedCoordsT mapped_coords(mu, mu);
  result += w * functor(mapped_coords, nodes);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Triag3DLagrangeP1Suite, Triag3DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Area )
{
  NodesT nodes_triag3D;
  nodes_triag3D <<
    0.0, 0.0, 0.0,
    1.0, 0.0, 1.0,
    1.0, 1.0, 1.0;
  BOOST_CHECK_EQUAL(Triag3DLagrangeP1::area(nodes_triag3D), std::sqrt(2.)/2.);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const SFT::ShapeFunctionsT reference_result(0.1, 0.1, 0.8);
  SFT::ShapeFunctionsT result;
  Triag3DLagrangeP1::shape_function_value(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
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
  Triag3DLagrangeP1::shape_function_gradient(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  SFT::JacobianT expected;
  expected(KSI,XX) = 0.6;
  expected(KSI,YY) = 0.9;
  expected(KSI,ZZ) = 0.5;

  expected(ETA,XX) = 0.3;
  expected(ETA,YY) = 1.8;
  expected(ETA,ZZ) = 1.;

  SFT::JacobianT result;
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

  gauss_integrate<1, GeoShape::TRIAG>(ftor, ftor.mapped_coords, result);

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
  CTable<Real> coordinates(Mesh::Tags::coordinates());
  CTable<Uint> connectivity("connectivity");
  create_cylinder(coordinates, connectivity, radius, u_segments, v_segments, height);

  // Check the area
  Real area = 0.;
  integrate_region(area, NormalVectorNorm(), coordinates, connectivity);
  BOOST_CHECK_CLOSE(area, 2.*MathConsts::pi()*radius*height, 0.1);

  // Flux from a constant vector field through a closed surface should be 0
  Real zero_flux = 0.;
  const SFT::CoordsT field_vector(0.35, 1.25, 0.);
  integrate_region(zero_flux, ConstVectorField(field_vector), coordinates, connectivity);
  BOOST_CHECK_SMALL(zero_flux, 1e-14);
}

BOOST_AUTO_TEST_CASE( ArcIntegral )
{
  // half cylinder arc
  CTable<Real> arc_coordinates(Mesh::Tags::coordinates());
  CTable<Uint> arc_connectivity("connectivity");
  create_cylinder(arc_coordinates, arc_connectivity, 1., 100, 24, 3., 0., MathConsts::pi());
  Real arc_flux = 0.;
  const SFT::CoordsT y_vector(0., 1., 0.);
  integrate_region(arc_flux, ConstVectorField(y_vector), arc_coordinates, arc_connectivity);
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
  CTable<Real> coordinates(Mesh::Tags::coordinates());
  CTable<Uint> connectivity("connectivity");
  create_cylinder(coordinates, connectivity, radius, u_segments, v_segments, height);

  // Rotating cylinder in uniform flow
  const Real u = 300.;
  const Real circulation = 975.;
  SFT::CoordsT force;
  force.setZero();
  integrate_region(force, RotatingCylinderPressure(radius, circulation, u), coordinates, connectivity);
  BOOST_CHECK_CLOSE(force[YY], height * 1.225*u*circulation, 0.001); // lift according to theory
  BOOST_CHECK_SMALL(force[XX], 1e-8); // Drag should be zero
  BOOST_CHECK_SMALL(force[ZZ], 1e-8);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

