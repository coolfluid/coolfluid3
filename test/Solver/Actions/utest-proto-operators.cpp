// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operators"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>


#include "Solver/CModel.hpp"
#include "Solver/CSolver.hpp"

#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Expression.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/NodeLooper.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

#include "common/Core.hpp"
#include "common/CRoot.hpp"
#include "common/Log.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementData.hpp"
#include "Mesh/FieldManager.hpp"
#include "Mesh/Geometry.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/ElementTypes.hpp"

#include "Physics/PhysModel.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"
#include "Tools/Testing/ProfiledTestFixture.hpp"

using namespace cf3;
using namespace cf3::Solver;
using namespace cf3::Solver::Actions;
using namespace cf3::Solver::Actions::Proto;
using namespace cf3::Mesh;
using namespace cf3::common;

using namespace cf3::Math::Consts;

using namespace boost;

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

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ProtoBasics )
{
  CMesh::Ptr mesh = Core::instance().root().create_component_ptr<CMesh>("rect");
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
  CMesh::Ptr mesh = Core::instance().root().create_component_ptr<CMesh>("line");
  Tools::MeshGeneration::create_line(*mesh, 1., 1);

  mesh->geometry().create_field( "solution", "Temperature" ).add_tag("solution");

  MeshTerm<0, ScalarField > temperature("Temperature", "solution");

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
}

BOOST_AUTO_TEST_CASE( RotatingCylinder )
{
  const Real radius = 1.;
  const Uint segments = 1000;
  const Real u = 300.;
  const Real circulation = 975.;
  const Real rho = 1.225;

  CMesh::Ptr mesh = Core::instance().root().create_component_ptr<CMesh>("circle");
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

  CMesh::Ptr mesh = Core::instance().root().create_component_ptr<CMesh>("circle");
  Tools::MeshGeneration::create_circle_2d(*mesh, radius, segments);

  mesh->geometry().create_field( "Pressure", "Pressure" ).add_tag("solution");

  MeshTerm<1, ScalarField > p("Pressure", "solution"); // Pressure field

  typedef boost::mpl::vector1< LagrangeP1::Line2D> SurfaceTypes;

  // Set a field with the pressures
  for_each_node
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
  CMesh::Ptr mesh = Core::instance().root().create_component_ptr<CMesh>("line");
  Tools::MeshGeneration::create_line(*mesh, 1., 1);

  mesh->geometry().create_field( "Temperature", "Temperature" ).add_tag("solution");

  MeshTerm<0, ScalarField > temperature("Temperature", "solution");

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
  CMesh::Ptr mesh = Core::instance().root().create_component_ptr<CMesh>("line2");
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
  CMesh::Ptr mesh = Core::instance().root().create_component_ptr<CMesh>("GaussQuadratureLine");
  Tools::MeshGeneration::create_line(*mesh, 1., 1);

  mesh->geometry().create_field("Temperature", "Temperature").add_tag("solution");

  MeshTerm<0, ScalarField > temperature("Temperature", "solution");

  RealVector1 mapped_coords;
  mapped_coords.setZero();

  RealMatrix2 exact; exact << 1., -1., -1., 1;
  RealMatrix2 result;
  RealMatrix2 zero;  zero.setZero();

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    group <<
    (
      boost::proto::lit(result) = zero,
      element_quadrature( boost::proto::lit(result) += transpose(nabla(temperature))*nabla(temperature) )
    )
  );

  check_close(result, exact, 1e-10);

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    group <<
    (
      boost::proto::lit(result) = zero,
      element_quadrature <<
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
  CMesh::Ptr mesh = Core::instance().root().create_component_ptr<CMesh>("GaussQuadratureLine");
  Tools::MeshGeneration::create_line(*mesh, 1., 1);

  mesh->geometry().create_field("Temperature", "Temperature").add_tag("solution");

  Real total = 0;

  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    mesh->topology(),
    group <<
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


/// Custom op taking indices as argument
struct IndexPrinter
{
  typedef void result_type;

  template<typename I, typename J>
  void operator()(const I, const J)
  {
    std::cout << "I is " << I::value << ", J is " << J::value << std::endl;
  }
};

static MakeSFOp<IndexPrinter>::type const print_indices = {};

BOOST_AUTO_TEST_CASE(IndexLooper)
{
  CMesh::Ptr mesh = Core::instance().root().create_component_ptr<CMesh>("QuadGrid");
  Tools::MeshGeneration::create_rectangle(*mesh, 1., 1., 1, 1);

  const RealVector2 idx(1.,2.);

  int result_i = 0;
  int result_j = 0;
  int result_ij = 0;

  for_each_element< boost::mpl::vector1<LagrangeP1::Quad2D> >
  (
    mesh->topology(),
    group <<
    (
      _cout << "i: " << _i << ", j: " << _j << "\n",
      print_indices(_i, _j),
      boost::proto::lit(result_i) += boost::proto::lit(idx)[_i], // Loop with _i in (0, 1)
      boost::proto::lit(result_j) += boost::proto::lit(idx)[_j], // Loop with _j in (0, 1)
      boost::proto::lit(result_ij) += boost::proto::lit(idx)[_i] + boost::proto::lit(idx)[_j] // Nested loop forall(_i): forall(_j)
    )
  );

  BOOST_CHECK_EQUAL(result_i, 3);
  BOOST_CHECK_EQUAL(result_j, 3);
  BOOST_CHECK_EQUAL(result_ij, 12);

  CMesh& line = Core::instance().root().create_component<CMesh>("Line");
  Tools::MeshGeneration::create_line(line, 1., 1);

  MeshTerm<0, VectorField> u("Velocity", "solution");
  line.geometry().create_field( "solution", "Velocity[v]" ).add_tag("solution");

  RealVector1 center;
  center.setZero();

  RealVector2 result1d;
  result1d.setZero();


  for_each_element< boost::mpl::vector1<LagrangeP1::Line1D> >
  (
    line.topology(),
    group <<
    (
      _cout << "i: " << _i << ", j: " << _j << "\n",
      result1d += nabla(u, center)[_i]
    )
  );

  BOOST_CHECK_CLOSE(result1d[0], -1., 1e-6);
  BOOST_CHECK_CLOSE(result1d[1], 1., 1e-6);
}

BOOST_AUTO_TEST_CASE( VectorMultiplication )
{
  MeshTerm<0, VectorField> u("Velocity", "solution");

  CModel& model = Core::instance().root().create_component<CModel>("Model");
  CDomain& dom = model.create_domain("Domain");
  CMesh& mesh = dom.create_component<CMesh>("QuadGrid2");
  Tools::MeshGeneration::create_rectangle(mesh, 1., 1., 1, 1);

  Physics::PhysModel& physics = model.create_physics("CF.Physics.DynamicModel");
  physics.configure_option(common::Tags::dimension(), 2u);

  // Create the initialization expression
  Expression::Ptr init = nodes_expression(u = coordinates);

  FieldManager& field_manager = dom.create_component<FieldManager>("FieldManager");
  field_manager.configure_option("variable_manager", physics.variable_manager().uri());

  // set up fields
  init->register_variables(physics);
  field_manager.create_field("solution", mesh.geometry());

  // Do the initialization
  init->loop(mesh.topology());

  RealVector4 result;
  result.setZero();

  // Run a vector product
  elements_expression
  (
    boost::mpl::vector1<LagrangeP1::Quad2D>(),
    element_quadrature(boost::proto::lit(result) += u*nabla(u))
  )->loop(mesh.topology());

  std::cout << result << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
