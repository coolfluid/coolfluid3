// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Line2D shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/FindComponents.hpp"

#include "math/Consts.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Space.hpp"

#include "mesh/Integrators/Gauss.hpp"

#include "mesh/LagrangeP1/Line2D.hpp"



#include "Tools/Testing/Difference.hpp"
#include "Tools/MeshGeneration/MeshGeneration.hpp"

using namespace boost::assign;
using namespace cf3;
using namespace cf3::common;
using namespace cf3::math;
using namespace cf3::mesh;
using namespace cf3::mesh::Integrators;
using namespace cf3::mesh::LagrangeP1;
using namespace cf3::Tools::Testing;
using namespace cf3::Tools::MeshGeneration;

//////////////////////////////////////////////////////////////////////////////

typedef Line2D ETYPE;

struct LagrangeP1Line2DFixture
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  typedef ETYPE::NodesT NodesT;
  /// common setup for each test case
  LagrangeP1Line2DFixture() :
    mapped_coords((ETYPE::MappedCoordsT() << .2).finished()),
    nodes
    (
      (ETYPE::NodesT() << 5., 7.,
                           10., 3.).finished()
    )
  {
  }

  const ETYPE::MappedCoordsT mapped_coords;
  const ETYPE::NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const ETYPE::NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      ETYPE::JacobianT jac;
      ETYPE::compute_jacobian(mapped_coords, m_nodes, jac);
      return sqrt(jac[0]*jac[0] + jac[1]*jac[1]);
    }
    ETYPE::MappedCoordsT mapped_coords;
  private:
    const ETYPE::NodesT& m_nodes;
  };

  /// Returns the norm of the normal vector to the curve or surface element (equal to tangent in the case of Line2D)
  struct NormalVectorNorm
  {
    Real operator()(const ETYPE::MappedCoordsT& mapped_coords, const ETYPE::NodesT& nodes)
    {
      ETYPE::CoordsT result;
      ETYPE::normal(mapped_coords, nodes, result);
      return result.norm();
    }
  };

  /// Returns the scalar product of a constant vector field and the local element normal
  struct ConstVectorField {

    ConstVectorField(const ETYPE::CoordsT& vector) : m_vector(vector) {}

    Real operator()(const ETYPE::MappedCoordsT& mapped_coords, const ETYPE::NodesT& nodes)
    {
      ETYPE::CoordsT normal;
      ETYPE::normal(mapped_coords, nodes, normal);
      return normal.dot(m_vector);
    }

  private:
    const ETYPE::CoordsT& m_vector;

  };

  /// Returns the static pressure around a rotating cylinder in a horizontal,
  /// uniform velocity field U, multiplied with the local normal vector
  struct RotatingCylinderPressure {

    RotatingCylinderPressure(const Real radius, const Real circulation, const Real U) :
      m_radius(radius), m_circulation(circulation), m_u(U) {}

    ETYPE::CoordsT operator()(const ETYPE::MappedCoordsT& mapped_coords, const ETYPE::NodesT& nodes)
    {
      // The pressures to interpolate
      ETYPE::CoordsT nodal_p;
      nodal_p[0] = pressure(theta(nodes.row(0)));
      nodal_p[1] = pressure(theta(nodes.row(1)));

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

const Real LagrangeP1Line2DFixture::RotatingCylinderPressure::m_rho = 1.225;

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

/// Integral for an element
template<typename ResultT, typename FunctorT, typename NodesT>
void integrate_element(ResultT& result, FunctorT functor, const NodesT& nodes)
{
  static const double mu = 0.;
  static const double w = 2.;
  static const ETYPE::MappedCoordsT mapped_coords((ETYPE::MappedCoordsT() << mu).finished());
  result += w * functor(mapped_coords, nodes);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Line2DSuite, LagrangeP1Line2DFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Area )
{
  ETYPE::NodesT nodes_line2D;
  nodes_line2D(0, XX) = 2.0;     nodes_line2D(0, YY) = 2.0;
  nodes_line2D(1, XX) = 1.0;     nodes_line2D(1, YY) = 1.0;
  BOOST_CHECK_EQUAL(Line2D::area(nodes_line2D),std::sqrt(2.));
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const ETYPE::SF::ValueT reference_result(0.4, 0.6);
  ETYPE::SF::ValueT result;
  Line2D::SF::compute_value(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  ETYPE::SF::GradientT result;
  ETYPE::SF::GradientT expected;
  expected(0,0) = -0.5;
  expected(0,1) = 0.5;
  Line2D::SF::compute_gradient(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  const ETYPE::JacobianT expected(2.5, -2.);
  ETYPE::JacobianT result;
  Line2D::compute_jacobian(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  ConstFunctor ftor(nodes);
  const Real vol = Line2D::area(nodes);

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
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("surface_integral");
  create_circle_2d(mesh, 1., 100);
  Table<Real>& coordinates = find_component_recursively<Dictionary>(mesh).coordinates();
  Table<Uint>& connectivity = find_component_recursively<Elements>(mesh).geometry_space().connectivity();


  // Check the length, using the line integral of one times the norm of the tangent vector
  Real length = 0.;
  integrate_region(length, NormalVectorNorm(), coordinates, connectivity);
  BOOST_CHECK_CLOSE(length, 2.*Consts::pi(), 0.1);

  // Flux from a constant vector field through a closed surface should be 0
  Real zero_flux = 0.;
  const ETYPE::CoordsT field_vector(0.35, 1.25);
  integrate_region(zero_flux, ConstVectorField(field_vector), coordinates, connectivity);
  BOOST_CHECK_SMALL(zero_flux, 1e-14);
}

BOOST_AUTO_TEST_CASE( ArcIntegral )
{
  // half circle arc, so the flux of a uniform field of unit vectors should equal the diameter
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("arc_integral");
  create_circle_2d(mesh, 1., 100, 0., Consts::pi());
  Table<Real>& arc_coordinates = find_component_recursively<Dictionary>(mesh).coordinates();
  Table<Uint>& arc_connectivity = find_component_recursively<Elements>(mesh).geometry_space().connectivity();
  Real arc_flux = 0.;
  const ETYPE::CoordsT y_vector(0., 1.);
  integrate_region(arc_flux, ConstVectorField(y_vector), arc_coordinates, arc_connectivity);
  BOOST_CHECK_CLOSE(arc_flux, 2., 0.01);
}

/// Lift produced by a rotating cylinder
BOOST_AUTO_TEST_CASE( RotatingCylinder )
{
  // Create an approximation of a circle
  const Real radius = 1.;
  const Uint segments = 10000;

  // complete circle
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("rotating_cylinder");
  create_circle_2d(mesh, 1., segments);
  Table<Real>& coordinates = find_component_recursively<Dictionary>(mesh).coordinates();
  Table<Uint>& connectivity = find_component_recursively<Elements>(mesh).geometry_space().connectivity();

  // Rotating cylinder in uniform flow
  const Real u = 300.;
  const Real circulation = 975.;
  ETYPE::CoordsT force(0.,0.);
  integrate_region(force, RotatingCylinderPressure(radius, circulation, u), coordinates, connectivity);
  BOOST_CHECK_CLOSE(force[YY], 1.225*u*circulation, 0.001); // lift according to theory
  BOOST_CHECK_SMALL(force[XX], 1e-8); // Drag should be zero
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

