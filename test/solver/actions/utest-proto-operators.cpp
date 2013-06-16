// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operators"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "solver/Model.hpp"
#include "solver/Solver.hpp"

#include "solver/actions/Proto/ElementLooper.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/Functions.hpp"
#include "solver/actions/Proto/NodeLooper.hpp"
#include "solver/actions/Proto/Terminals.hpp"

#include "common/Core.hpp"
#include "common/Log.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Dictionary.hpp"

#include "mesh/Integrators/Gauss.hpp"
#include "mesh/ElementTypes.hpp"

#include "physics/PhysModel.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::mesh;
using namespace cf3::common;

using namespace cf3::math::Consts;

/// Check close, for testing purposes
inline void check_close(const RealMatrix2& a, const RealMatrix2& b, const Real threshold)
{
  for(Uint i = 0; i != a.rows(); ++i)
    for(Uint j = 0; j != a.cols(); ++j)
      BOOST_CHECK_CLOSE(a(i,j), b(i,j), threshold);
}

static boost::proto::terminal< void(*)(const RealMatrix2&, const RealMatrix2&, Real) >::type const _check_close = {&check_close};

////////////////////////////////////////////////////

/// List of all supported shapefunctions that allow high order integration
typedef boost::mpl::vector3< LagrangeP1::Line1D,
                             LagrangeP1::Quad2D,
                             LagrangeP1::Hexa3D
> HigherIntegrationElements;

typedef boost::mpl::vector5< LagrangeP1::Line1D,
                             LagrangeP1::Triag2D,
                             LagrangeP1::Quad2D,
                             LagrangeP1::Hexa3D,
                             LagrangeP1::Tetra3D
> VolumeTypes;

BOOST_AUTO_TEST_SUITE( ProtoOperatorsSuite )

using boost::proto::lit;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ProtoBasics )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("rect");
  Tools::MeshGeneration::create_rectangle(*mesh, 5, 5, 5, 5);

  RealVector center_coords(2);
  center_coords.setZero();

  // Use the volume function
  for_each_element<VolumeTypes>(mesh->topology(), _cout << "Volume = " << volume << ", centroid = " << transpose(coordinates(center_coords)) << "\n");
  std::cout << std::endl; // Can't be in expression

  // volume calculation
  Real vol1 = 0.;
  for_each_element<VolumeTypes>(mesh->topology(), vol1 += volume);

  CFinfo << "Mesh volume: " << vol1 << CFendl;

  // For an all-quad mesh, this is the same... cool or what?
  Real vol2 = 0.;
  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    vol2 += 0.5*((nodes[2][0] - nodes[0][0]) * (nodes[3][1] - nodes[1][1])
              -  (nodes[2][1] - nodes[0][1]) * (nodes[3][0] - nodes[1][0]))
  );
  BOOST_CHECK_CLOSE(vol1, vol2, 1e-5);
}

BOOST_AUTO_TEST_CASE( MatrixProducts )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("line");
  Tools::MeshGeneration::create_line(*mesh, 1., 1);

  mesh->geometry_fields().create_field( "solution", "Temperature" ).add_tag("solution");

  FieldVariable<0, ScalarField > temperature("Temperature", "solution");

  for_each_node(mesh->topology(), temperature = 5.);

  RealVector1 mapped_coords;
  mapped_coords.setZero();

  RealMatrix2 exact; exact << 1., -1., -1., 1;
  RealMatrix2 result;

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    boost::proto::lit(result) = 0.5 * integral<1>(transpose(nabla(temperature))*nabla(temperature)) * integral<1>(transpose(nabla(temperature))*nabla(temperature)) * transpose(nabla(temperature, mapped_coords)) * nabla(temperature, mapped_coords)
  );

  check_close(result, 8*exact, 1e-10);

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    boost::proto::lit(result) = integral<1>(transpose(nabla(temperature))*nabla(temperature)) * 0.5
  );

  check_close(result, exact, 1e-10);

  result.setZero();

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    element_quadrature( boost::proto::lit(result) += transpose(N(temperature))*N(temperature) * (1. / temperature) * boost::proto::lit(3.) )
  );

  exact << 0.2, 0.1, 0.1, 0.2;
  check_close(result, exact, 1e-10);
}

BOOST_AUTO_TEST_CASE( RotatingCylinder )
{
  const Real radius = 1.;
  const Uint segments = 1000;
  const Real u = 300.;
  const Real circulation = 975.;
  const Real rho = 1.225;

  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("circle");
  Tools::MeshGeneration::create_circle_2d(*mesh, radius, segments);

  typedef boost::mpl::vector1< LagrangeP1::Line2D> SurfaceTypes;

  RealVector2 force;
  force.setZero();

  for_each_element<SurfaceTypes>
  (
    mesh->topology(),
    force += integral<1>
    (
      pow<2>
      (
        2. * u * _sin( _atan2(coordinates[1], coordinates[0]) ) + circulation / (2. * pi() * radius)
      )  * 0.5 * rho * normal
    )
  );

  BOOST_CHECK_CLOSE(force[YY], rho*u*circulation, 0.001); // lift according to theory
  BOOST_CHECK_SMALL(force[XX], 1e-8); // Drag should be zero
}

/// First create a field with the pressure distribution, then integrate it
BOOST_AUTO_TEST_CASE( RotatingCylinderField )
{
  const Real radius = 1.;
  const Uint segments = 1000;
  const Real u = 300.;
  const Real circulation = 975.;
  const Real rho = 1.225;

  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("circle");
  Tools::MeshGeneration::create_circle_2d(*mesh, radius, segments);

  mesh->geometry_fields().create_field( "Pressure", "Pressure" ).add_tag("solution");

  FieldVariable<1, ScalarField > p("Pressure", "solution"); // Pressure field

  typedef boost::mpl::vector1< LagrangeP1::Line2D> SurfaceTypes;

  // Set a field with the pressures
  for_each_node<2>
  (
    mesh->topology(),
    p += pow<2>
    (
      2. * u * _sin( _atan2(coordinates[1], coordinates[0]) ) + circulation / (2. * pi() * radius)
    )  * 0.5 * rho
  );

  RealVector2 force;
  force.setZero();

  RealVector1 mc;
  mc.setZero();

  for_each_element<SurfaceTypes>
  (
    mesh->topology(),
    force += integral<1>(p * normal)
  );

  BOOST_CHECK_CLOSE(force[YY], rho*u*circulation, 0.001); // lift according to theory
  BOOST_CHECK_SMALL(force[XX], 1e-8); // Drag should be zero
}

struct CustomLaplacian
{
  /// Custom ops must implement the  TR1 result_of protocol
  template<typename Signature>
  struct result;

  template<typename This, typename FieldDataT>
  struct result<This(FieldDataT)>
  {
    typedef const Eigen::Matrix<Real, FieldDataT::dimension*FieldDataT::EtypeT::nb_nodes, FieldDataT::dimension*FieldDataT::EtypeT::nb_nodes>& type;
  };

  template<typename StorageT, typename FieldDataT>
  const StorageT& operator()(StorageT& result, const FieldDataT& field) const
  {
    typedef typename FieldDataT::EtypeT EtypeT;
    const Eigen::Matrix<Real, EtypeT::nb_nodes, EtypeT::nb_nodes> m = field.nabla().transpose() * field.nabla();
    for(Uint d = 0; d != FieldDataT::dimension; ++d)
    {
      result.template block<EtypeT::nb_nodes, EtypeT::nb_nodes>(EtypeT::nb_nodes*d, EtypeT::nb_nodes*d).noalias() = m;
    }
    return result;
  }
};

MakeSFOp<CustomLaplacian>::type laplacian_cust = {};

BOOST_AUTO_TEST_CASE( CustomOp )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("line");
  Tools::MeshGeneration::create_line(*mesh, 1., 1);

  mesh->geometry_fields().create_field( "Temperature", "Temperature" ).add_tag("solution");

  FieldVariable<0, ScalarField > temperature("Temperature", "solution");

  RealMatrix2 exact; exact << 1., -1., -1., 1;
  RealMatrix2 result;

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    boost::proto::lit(result) = integral<1>(laplacian_cust(temperature)) * 0.5
  );

  check_close(result, exact, 1e-10);

}

/// Custom op that just modifies its argument
struct Counter
{
  /// Dummy result
  typedef void result_type;

  result_type operator()(int& arg) const
  {
    ++arg;
  }
};

MakeSFOp<Counter>::type counter = {};

/// Test a custom operator that modifies its arguments
BOOST_AUTO_TEST_CASE( VoidOp )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("line2");
  Tools::MeshGeneration::create_line(*mesh, 1., 10);

  // Check if the counter really counts
  int count = 0;
  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    counter(count)
  );

  BOOST_CHECK_EQUAL(count, 10);
}

BOOST_AUTO_TEST_CASE( ElementGaussQuadrature )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("GaussQuadratureLine");
  Tools::MeshGeneration::create_line(*mesh, 1., 1);

  mesh->geometry_fields().create_field("Temperature", "Temperature").add_tag("solution");

  FieldVariable<0, ScalarField > temperature("Temperature", "solution");

  RealVector1 mapped_coords;
  mapped_coords.setZero();

  RealMatrix2 exact; exact << 1., -1., -1., 1;
  RealMatrix2 result;
  RealMatrix2 zero;  zero.setZero();

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    group
    (
      boost::proto::lit(result) = zero,
      element_quadrature( boost::proto::lit(result) += transpose(nabla(temperature))*nabla(temperature) )
    )
  );

  check_close(result, exact, 1e-10);

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    group
    (
      boost::proto::lit(result) = zero,
      element_quadrature
      (
        boost::proto::lit(result) += transpose(nabla(temperature))*nabla(temperature),
        boost::proto::lit(result) += transpose(nabla(temperature))*nabla(temperature)
      )
    )
  );

  check_close(result, 2.*exact, 1e-10);
}


BOOST_AUTO_TEST_CASE(GroupArity)
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("GaussQuadratureLine");
  Tools::MeshGeneration::create_line(*mesh, 1., 1);

  mesh->geometry_fields().create_field("Temperature", "Temperature").add_tag("solution");

  Real total = 0;

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    group
    (
      boost::proto::lit(total) += volume,
      boost::proto::lit(total) += volume
    )
  );

  BOOST_CHECK_CLOSE(total, 2., 1e-10);
}

struct IntegralConstantTag {};

template<int I>
struct IntegralConstantGrammar :
  boost::proto::when
  <
    boost::proto::terminal<IntegralConstantTag>,
    boost::proto::_make_terminal(boost::mpl::int_<I>())
  >
{
};

/// Test if we can replace an expression with a tag using integral constants
BOOST_AUTO_TEST_CASE(IntegralConstant)
{
  boost::proto::terminal<IntegralConstantTag>::type integral_const;
  BOOST_CHECK_EQUAL(boost::proto::value(IntegralConstantGrammar<3>()(integral_const)), 3);
}

BOOST_AUTO_TEST_CASE(IndexLooper)
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("QuadGrid");
  Tools::MeshGeneration::create_rectangle(*mesh, 1., 1., 1, 1);

  const RealVector2 idx(1.,2.);

  Real result_i = 0;
  Real result_j = 0;
  Real result_ij = 0;

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      boost::proto::lit(result_i) += boost::proto::lit(idx)[_i], // Loop with _i in (0, 1)
      boost::proto::lit(result_j) += boost::proto::lit(idx)[_j], // Loop with _j in (0, 1)
      boost::proto::lit(result_ij) += boost::proto::lit(idx)[_i] + boost::proto::lit(idx)[_j] // Nested loop forall(_i): forall(_j)
    )
  );

  BOOST_CHECK_EQUAL(result_i, 3.);
  BOOST_CHECK_EQUAL(result_j, 3.);
  BOOST_CHECK_EQUAL(result_ij, 12.);

  Mesh& line = *Core::instance().root().create_component<Mesh>("Line");
  Tools::MeshGeneration::create_line(line, 1., 1);

  FieldVariable<0, VectorField> u("Velocity", "solution");
  line.geometry_fields().create_field( "solution", "Velocity[v]" ).add_tag("solution");

  RealVector1 center;
  center.setZero();

  RealVector2 result1d;
  result1d.setZero();


  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    line.topology(),
    group
    (
      group(_cout << "i: " << _i << ", j: " << _j << "\n"),
      result1d += nabla(u, center)[_i]
    )
  );

  BOOST_CHECK_CLOSE(result1d[0], -1., 1e-6);
  BOOST_CHECK_CLOSE(result1d[1], 1., 1e-6);
}

BOOST_AUTO_TEST_CASE( VectorMultiplication )
{
  FieldVariable<0, VectorField> u("Velocity", "solution");
  FieldVariable<0, ScalarField> T("Temperature", "solution");

  Model& model = *Core::instance().root().create_component<Model>("Model");
  Domain& dom = model.create_domain("Domain");
  Mesh& mesh = *dom.create_component<Mesh>("QuadGrid2");
  Tools::MeshGeneration::create_rectangle(mesh, 1., 1., 5, 5);

  physics::PhysModel& physics = model.create_physics("cf3.physics.DynamicModel");
  physics.options().set(common::Tags::dimension(), 2u);

  // Create the initialization expression
  boost::shared_ptr< Expression > init = nodes_expression(u = coordinates);
  boost::shared_ptr< Expression > init_temp = nodes_expression(T = coordinates[0]);

  FieldManager& field_manager = *dom.create_component<FieldManager>("FieldManager");
  field_manager.options().set("variable_manager", physics.variable_manager().handle<math::VariableManager>());

  // set up fields
  init->register_variables(physics);
  init_temp->register_variables(physics);
  field_manager.create_field("solution", mesh.geometry_fields());

  // Do the initialization
  init->loop(mesh.topology());
  init_temp->loop(mesh.topology());

  RealVector4 result;
  result.setZero();

  // Run a vector product
  elements_expression
  (
    boost::mpl::vector1<LagrangeP1::Quad2D>(),
    element_quadrature(boost::proto::lit(result) += u*nabla(u))
  )->loop(mesh.topology());

  std::cout << "advection: " << result.transpose() << std::endl;

  //RealMatrix4 gradient_result; gradient_result.setZero();
  Real gradient_result = 0.;
  // Gradient calculation for a scalar
  elements_expression
  (
    boost::mpl::vector1<LagrangeP1::Quad2D>(),
   element_quadrature(boost::proto::lit(gradient_result) += _norm(transpose(N(T))*transpose(nabla(T)*nodal_values(T))*nabla(T)))
  )->loop(mesh.topology());

  std::cout << "gradient: " << gradient_result << std::endl;
}


BOOST_AUTO_TEST_CASE( NodeExprGrouping )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("line2");
  Tools::MeshGeneration::create_line(*mesh, 1., 4);

  mesh->geometry_fields().create_field( "solution", "Temperature" ).add_tag("solution");

  FieldVariable<0, ScalarField > T("Temperature", "solution");
  Real total = 0.;

  boost::shared_ptr< Expression > test_expr = nodes_expression
  (
    group
    (
      T = 6.,
      T += 4.,
      _cout << T << "\n",
      boost::proto::lit(total) += T
    )
  );

  test_expr->loop(mesh->topology());

  BOOST_CHECK_EQUAL(total, 50.);
}

BOOST_AUTO_TEST_CASE( NodeExprFunctionParsing )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("line3");
  Tools::MeshGeneration::create_line(*mesh, 4., 4);

  mesh->geometry_fields().create_field( "solution", "Temperature" ).add_tag("solution");

  FieldVariable<0, VectorField > T("Temperature", "solution");
  RealVector total(1); total.setZero();

  solver::actions::Proto::VectorFunction f;
  f.variables("x");
  f.functions(std::vector<std::string>(1, "x+1"));
  f.parse();
  f.predefined_values.resize(1);

  boost::shared_ptr< Expression > test_expr = nodes_expression
  (
    group
    (
      T = boost::proto::lit(f),
      _cout << T << "\n",
      boost::proto::lit(total) += T
    )
  );

  test_expr->loop(mesh->topology());

  BOOST_CHECK_EQUAL(total[0], 15.);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ProtoAccumulators )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("line4");
  Tools::MeshGeneration::create_line(*mesh, 4., 400);

  boost::accumulators::accumulator_set< Real, boost::accumulators::stats<boost::accumulators::tag::mean, boost::accumulators::tag::max> > acc;

  mesh->geometry_fields().create_field( "solution", "Temperature" ).add_tag("solution");
  FieldVariable<0, VectorField > T("Temperature", "solution");

  boost::shared_ptr<Expression> init_expr = nodes_expression(T = coordinates);
  init_expr->loop(mesh->topology());

  boost::shared_ptr<Expression> test_expr = nodes_expression(boost::proto::lit(acc)(_norm(T)));

  test_expr->loop(mesh->topology());

  BOOST_CHECK_EQUAL(boost::accumulators::max(acc), 4.);
  BOOST_CHECK_CLOSE(boost::accumulators::mean(acc), 2., 1e-6);
}

BOOST_AUTO_TEST_CASE( AssignMatrix )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("line5");
  Tools::MeshGeneration::create_line(*mesh, 1., 1);

  mesh->geometry_fields().create_field( "Temperature", "Temperature" ).add_tag("solution");

  FieldVariable<0, ScalarField > T("Temperature", "solution");

  RealMatrix tmp;

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    boost::proto::lit(tmp) = integral<1>(transpose(nabla(T))*nabla(T))
  );

  std::cout << "tmp=\n" << tmp << std::endl;

}

BOOST_AUTO_TEST_CASE( GaussPointAccess )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("gauss_rect");
  Tools::MeshGeneration::create_rectangle(*mesh, 5., 5., 2, 2);

  RealVector center_coords(2);
  center_coords.setZero();

  for_each_element<VolumeTypes>(mesh->topology(), _cout << "point = " << transpose(gauss_points_1) << ", weight = " << gauss_weights_1 << "\n");
  std::cout << std::endl;

  for_each_element<VolumeTypes>(mesh->topology(), _cout << "point = " << transpose(gauss_points_2) << ", weight = " << gauss_weights_2 << "\n");
  std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE( RestrictToEtype )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("rect_etypecheck");
  Tools::MeshGeneration::create_rectangle(*mesh, 5., 5., 5, 2);

  boost::proto::terminal< RestrictToElementTypeTag< boost::mpl::vector1<LagrangeP1::Quad2D> > >::type quads_only;
  boost::proto::terminal< RestrictToElementTypeTag< boost::mpl::vector1<LagrangeP1::Line2D> > >::type lines_only;

  Uint nb_cells = 0;
  Uint nb_faces = 0;

  for_each_element< boost::mpl::vector2<LagrangeP1::Quad2D, LagrangeP1::Line2D> >(mesh->topology(),
    group
    (
      quads_only(boost::proto::lit(nb_cells) += 1),
      lines_only(boost::proto::lit(nb_faces) += 1)
    )
  );

  BOOST_CHECK_EQUAL(nb_cells, 10);
  BOOST_CHECK_EQUAL(nb_faces, 16);

  std::cout << "mesh has " << nb_cells << " cells and " << nb_faces << " faces" << std::endl;
}

BOOST_AUTO_TEST_CASE( AddElementValues )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("add_elems_mesh");
  Tools::MeshGeneration::create_rectangle(*mesh, 6., 3., 3, 3);

  mesh->geometry_fields().create_field( "solution", "Temperature[v]" ).add_tag("solution");

  FieldVariable<0, VectorField > T("Temperature", "solution");

  Eigen::Matrix<Real, 8, 8> vals; vals.setConstant(0.125);

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      lump(vals),
      T += diagonal(vals)
    )
  );

  Real check = 0;
  for_each_node(mesh->topology(), group(boost::proto::lit(check) += T[_i], _cout << "summed nodal value: " << transpose(T) << "\n"));

  for_each_node(mesh->topology(), T[_i] = 0.);

  BOOST_CHECK_EQUAL(check, 72);

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group
    (
      T[_i] += nabla(T, gauss_points_1)[_i],
      _cout << nabla(T, gauss_points_1)[_i] << "\n"
    )
  );

  Real x_check = 0.;
  Real y_check = 0.;

  for_each_node<2>(mesh->topology(), group
  (
    boost::proto::lit(x_check) += T[0],
    boost::proto::lit(y_check) += T[1],
    _cout << "checked nodal value: " << transpose(T) << " at (" << transpose(coordinates) << ")\n"
  ));

  BOOST_CHECK_EQUAL(x_check, 0.);
  BOOST_CHECK_EQUAL(y_check, 0.);
}


BOOST_AUTO_TEST_CASE( NodeIndexLoop )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("ArrayOpsGrid");
  Tools::MeshGeneration::create_rectangle(*mesh, 1., 1., 1, 1);

  FieldVariable<0, VectorField> u("Velocity", "solution");
  mesh->geometry_fields().create_field( "solution", "Velocity[v]" ).add_tag("solution");

  RealVector v(2); v << 2. , 4.;
  RealVector v2(2); v2.setZero();

  for_each_node
  (
    mesh->topology(),
    group
    (
      lit(v2)[_i] += lit(v)[_i],
      u[_i] = lit(v)[_i]
    )
  );

  BOOST_CHECK_EQUAL(v2[0], 8.);
  BOOST_CHECK_EQUAL(v2[1], 16.);

  Real x_sum = 0.;
  Real y_sum = 0.;
  Real total_sum = 0.;

  for_each_node
  (
    mesh->topology(),
    group
    (
      lit(x_sum) += u[0],
      lit(y_sum) += u[1],
      lit(total_sum) += u[_i]
    )
  );

  BOOST_CHECK_EQUAL(x_sum, 8.);
  BOOST_CHECK_EQUAL(y_sum, 16.);
  BOOST_CHECK_EQUAL(total_sum, 24.);
}

BOOST_AUTO_TEST_CASE( ElementVector )
{
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("ElementVector");
  Tools::MeshGeneration::create_rectangle(*mesh, 1., 1., 1, 1);

  FieldVariable<0, VectorField> u("Velocity", "solution");
  mesh->geometry_fields().create_field( "solution", "Velocity[v]" ).add_tag("solution");

  for_each_node
  (
    mesh->topology(),
    u = coordinates
  );

  RealVector elvec;

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    lit(elvec) = element_vector(u)
  );

  std::cout << elvec.transpose() << std::endl;
  BOOST_CHECK_EQUAL(elvec.rows(), 8);
  RealVector ref(8);
  ref << 0,1,1,0,0,0,1,1;
  BOOST_CHECK_EQUAL(elvec, ref);
}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
