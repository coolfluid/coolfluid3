// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the ETYPE shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"

#include "math/Consts.hpp"
#include "math/Functions.hpp"

#include "common/Table.hpp"
#include "common/Table.hpp"
#include "mesh/Integrators/Gauss.hpp"
#include "mesh/LagrangeP1/Quad3D.hpp"
#include "mesh/ElementData.hpp"


#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::math;
using namespace cf3::math::Consts;
using namespace cf3::mesh;
using namespace cf3::mesh::Integrators;
using namespace cf3::mesh::LagrangeP1;
using namespace cf3::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Quad3D ETYPE;

struct LagrangeP1Quad3DFixture
{
  typedef ETYPE::NodesT NodesT;
  /// common setup for each test case
  LagrangeP1Quad3DFixture() : mapped_coords(0.1, 0.8), nodes((NodesT() << 0.5,0.3,-1.,
                                                                          1.1,1.2,0.,
                                                                          1.35,1.9,1.,
                                                                          0.8,2.1,0.).finished())
  {
  }

  /// common tear-down for each test case
  ~LagrangeP1Quad3DFixture()
  {
  }

  /// Fills the given coordinate and connectivity data to create a cylinder along the Z-axis, consisting of ETYPE elements
  void create_cylinder(Table<Real>& coordinates, Table<Uint>& connectivity, const Real radius, const Uint u_segments, const Uint v_segments, const Real height, const Real start_angle = 0., const Real end_angle = 2.*Consts::pi())
  {
    const Uint dim = ETYPE::dimension;
    const Uint nb_nodes = ETYPE::nb_nodes;
    const bool closed = std::abs(std::abs(end_angle - start_angle) - 2.0*Consts::pi()) < eps();

    coordinates.set_row_size(dim);
    Table<Real>::ArrayT& coord_array = coordinates.array();
    coord_array.resize(boost::extents[(u_segments + (!closed)) * (v_segments+1)][dim]);

    connectivity.set_row_size(nb_nodes);
    Table<Uint>::ArrayT& conn_array = connectivity.array();
    conn_array.resize(boost::extents[u_segments * v_segments][nb_nodes]);
    const Real v_step = height / v_segments;

    if(!closed)
    {
      for(Uint v = 0; v <= v_segments; ++v)
      {
        const Real z_coord = v_step * static_cast<Real>(v);
        for(Uint u = 0; u <= u_segments; ++u)
        {
          const Real theta = start_angle + (end_angle - start_angle) * (static_cast<Real>(u) / static_cast<Real>(u_segments));
          Table<Real>::Row coord_row = coord_array[v*(u_segments+1) + u];

          coord_row[XX] = radius * cos(theta);
          coord_row[YY] = radius * sin(theta);
          coord_row[ZZ] = z_coord;
        }
      }

      for(Uint v = 0; v != v_segments; ++v)
      {
        // const Real z_coord = v_step * static_cast<Real>(v);
        for(Uint u = 0; u != u_segments; ++u)
        {
          Table<Uint>::Row nodes = conn_array[v*u_segments + u];
          nodes[0] = v*(u_segments+1) + u;
          nodes[1] = nodes[0] + 1;
          nodes[3] = (v+1)*(u_segments+1) + u;
          nodes[2] = nodes[3] + 1;
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
          Table<Real>::Row coord_row = coord_array[v*u_segments + u];

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
          Table<Uint>::Row nodes = conn_array[v*u_segments + u];
          nodes[0] = v*u_segments + u;
          nodes[1] = nodes[0] + 1;
          nodes[3] = (v+1)*u_segments + u;
          nodes[2] = nodes[3] + 1;
        }
        conn_array[v*u_segments + u_segments-1][1] = v*u_segments;
        conn_array[v*u_segments + u_segments-1][2] = (v+1)*u_segments;
      }
    }
  }

  const ETYPE::MappedCoordsT mapped_coords;
  const NodesT nodes;

  template<typename SF>
  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      typename SF::CoordsT result;
      SF::normal(mapped_coords, m_nodes, result);
      return result.norm();
    }
    typename SF::MappedCoordsT mapped_coords;
  private:
    const NodesT& m_nodes;
  };

  /// Returns the norm of the normal vector to the curve or surface element (equal to tangent in the case of Line2D)
  struct NormalVectorNorm {

    Real operator()(const ETYPE::MappedCoordsT& mapped_coords, const NodesT& nodes)
    {
      ETYPE::CoordsT result;
      ETYPE::normal(mapped_coords, nodes, result);
      return result.norm();
    }

  };

  /// Returns the scalar product of a constant vector field and the local element normal
  struct ConstVectorField {

    ConstVectorField(const ETYPE::CoordsT& vector) : m_vector(vector) {}

    Real operator()(const ETYPE::MappedCoordsT& mapped_coords, const NodesT& nodes)
    {
      ETYPE::CoordsT normal;
      ETYPE::normal(mapped_coords, nodes, normal);
      return Functions::inner_product(normal, m_vector);
    }

  private:
    const ETYPE::CoordsT m_vector;
  };

  /// Returns the static pressure around a rotating cylinder in a horizontal,
  /// uniform velocity field U, multiplied with the local normal vector
  struct RotatingCylinderPressure
  {

    RotatingCylinderPressure(const Real radius, const Real circulation, const Real U) :
      m_radius(radius), m_circulation(circulation), m_u(U) {}

    ETYPE::CoordsT operator()(const RealVector& mapped_coords, const NodesT& nodes)
    {
      // The pressures to interpolate
      RealVector4 nodal_p;
      nodal_p[0] = pressure(theta(nodes.row(0)));
      nodal_p[1] = pressure(theta(nodes.row(1)));
      nodal_p[2] = pressure(theta(nodes.row(2)));
      nodal_p[3] = pressure(theta(nodes.row(3)));

      // The local normal
      ETYPE::CoordsT normal;
      ETYPE::normal(mapped_coords, nodes, normal);

      // Interpolate the pressure
      ETYPE::SF::ValueT sf_mat;
      ETYPE::SF::compute_value(mapped_coords, sf_mat);

      return normal * (sf_mat * nodal_p);
    }

  private:
    const Real m_radius;
    const Real m_circulation;
    const Real m_u;
    static const Real m_rho;

    // Reconstruct the value of theta, based on the coordinates
    Real theta(const ETYPE::CoordsT& coords)
    {
      return atan2(coords[YY], coords[XX]);
    }

    // Pressure in function of theta
    Real pressure(const Real theta)
    {
      Real tmp = (2. * m_u * sin(theta) + m_circulation / (2. * Consts::pi() * m_radius));
      return 0.5 * m_rho * tmp * tmp;
    }

  };
};

const Real LagrangeP1Quad3DFixture::RotatingCylinderPressure::m_rho = 1.225;

/// Integrate over a region
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

/// Integration over a single element
template<typename ResultT, typename FunctorT, typename NodesT>
void integrate_element(ResultT& result, FunctorT functor, const NodesT& nodes)
{
  static const double mu = 0.;
  static const double w = 4.;
  static const ETYPE::MappedCoordsT mapped_coords(mu, mu);
  result += w * functor(mapped_coords, nodes);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LagrangeP1Quad3DSuite, LagrangeP1Quad3DFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Area )
{
  ETYPE::NodesT nodes_quad3D;
  nodes_quad3D <<
    0.0, 0.0, 0.0,
    1.0, 0.0, 1.0,
    1.0, 1.0, 1.0,
    0.0, 1.0, 0.0;
  BOOST_CHECK_EQUAL(ETYPE::area(nodes_quad3D), std::sqrt(2.));
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const ETYPE::SF::ValueT reference_result(0.045, 0.055, 0.495, 0.405);
  ETYPE::SF::ValueT result;
  ETYPE::SF::compute_value(mapped_coords, result);
  cf3::Tools::Testing::Accumulator accumulator;
  cf3::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  ETYPE::SF::GradientT expected;
  const cf3::Real ksi  = mapped_coords[0];
  const cf3::Real eta = mapped_coords[1];
  expected(0,0) = 0.25 * (-1 + eta);
  expected(1,0) = 0.25 * (-1 + ksi);
  expected(0,1) = 0.25 * ( 1 - eta);
  expected(1,1) = 0.25 * (-1 - ksi);
  expected(0,2) = 0.25 * ( 1 + eta);
  expected(1,2) = 0.25 * ( 1 + ksi);
  expected(0,3) = 0.25 * (-1 - eta);
  expected(1,3) = 0.25 * ( 1 - ksi);
  ETYPE::SF::GradientT result;
  ETYPE::SF::compute_gradient(mapped_coords, result);
  cf3::Tools::Testing::Accumulator accumulator;
  cf3::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  ETYPE::JacobianT expected;
  expected(KSI,XX) = 0.2775;
  expected(KSI,YY) = -0.045;
  expected(KSI,ZZ) = 0.5;

  expected(ETA,XX) = 0.13625;
  expected(ETA,YY) = 0.5975;
  expected(ETA,ZZ) = 0.5;

  ETYPE::JacobianT result;
  ETYPE::compute_jacobian(mapped_coords, nodes, result);
  cf3::Tools::Testing::Accumulator accumulator;
  cf3::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 15);
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  ConstFunctor<ETYPE> ftor(nodes);
  const Real area = ETYPE::area(nodes);

  Real result1 = 0.0;
  Real result2 = 0.0;
  Real result4 = 0.0;
  Real result8 = 0.0;
  Real result16 = 0.0;
  Real result32 = 0.0;

  gauss_integrate<1, GeoShape::QUAD>(ftor, ftor.mapped_coords, result1);
  gauss_integrate<2, GeoShape::QUAD>(ftor, ftor.mapped_coords, result2);
  gauss_integrate<4, GeoShape::QUAD>(ftor, ftor.mapped_coords, result4);
  gauss_integrate<8, GeoShape::QUAD>(ftor, ftor.mapped_coords, result8);
  gauss_integrate<16, GeoShape::QUAD>(ftor, ftor.mapped_coords, result16);
  gauss_integrate<32, GeoShape::QUAD>(ftor, ftor.mapped_coords, result32);

  BOOST_CHECK_CLOSE(result1, area, 0.001);
  // TODO: Computed area is approximate, higher order integration is actually more accurate than what is returned by area()
//  BOOST_CHECK_CLOSE(result2, area, 0.001);
//  BOOST_CHECK_CLOSE(result4, area, 0.001);
//  BOOST_CHECK_CLOSE(result8, area, 0.001);
//  BOOST_CHECK_CLOSE(result16, area, 0.001);
//  BOOST_CHECK_CLOSE(result32, area, 0.001);
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
  boost::shared_ptr< Table<Real> > coordinates(common::allocate_component< Table<Real> >(mesh::Tags::coordinates()));
  boost::shared_ptr< Table<Uint> > connectivity(common::allocate_component< Table<Uint> >("connectivity"));
  create_cylinder(*coordinates, *connectivity, radius, u_segments, v_segments, height);

  // Check the area
  Real area = 0.;
  integrate_region(area, NormalVectorNorm(), *coordinates, *connectivity);
  BOOST_CHECK_CLOSE(area, 2.*Consts::pi()*radius*height, 0.1);

  // Flux from a constant vector field through a closed surface should be 0
  Real zero_flux = 0.;
  const ETYPE::CoordsT field_vector(0.35, 1.25, 0.);
  integrate_region(zero_flux, ConstVectorField(field_vector), *coordinates, *connectivity);
  BOOST_CHECK_SMALL(zero_flux, 1e-14);
}

BOOST_AUTO_TEST_CASE( ArcIntegral )
{
  // half cylinder arc
  boost::shared_ptr< Table<Real> > arc_coordinates(common::allocate_component< Table<Real> >(mesh::Tags::coordinates()));
  boost::shared_ptr< Table<Uint> > arc_connectivity(common::allocate_component< Table<Uint> >("connectivity"));
  create_cylinder(*arc_coordinates, *arc_connectivity, 1., 100, 24, 3., 0., Consts::pi());
  Real arc_flux = 0.;
  const ETYPE::CoordsT y_vector(0., 1., 0.);
  integrate_region(arc_flux, ConstVectorField(y_vector), *arc_coordinates, *arc_connectivity);
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
  boost::shared_ptr< Table<Real> > coordinates(common::allocate_component< Table<Real> >(mesh::Tags::coordinates()));
  boost::shared_ptr< Table<Uint> > connectivity(common::allocate_component< Table<Uint> >("connectivity"));
  create_cylinder(*coordinates, *connectivity, radius, u_segments, v_segments, height);

  // Rotating cylinder in uniform flow
  const Real u = 300.;
  const Real circulation = 975.;
  ETYPE::CoordsT force;
  force.setZero();
  integrate_region(force, RotatingCylinderPressure(radius, circulation, u), *coordinates, *connectivity);
  BOOST_CHECK_CLOSE(force[YY], height * 1.225*u*circulation, 0.001); // lift according to theory
  BOOST_CHECK_SMALL(force[XX], 1e-8); // Drag should be zero
  BOOST_CHECK_SMALL(force[ZZ], 1e-8);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

