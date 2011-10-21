// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operations related to components"

#include <map>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "common/CActionDirector.hpp"
#include "common/Core.hpp"
#include "common/CRoot.hpp"
#include "common/Log.hpp"
#include "common/TimedComponent.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/CDomain.hpp"
#include "mesh/CMesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CModel.hpp"
#include "Solver/CSimpleSolver.hpp"

#include "Solver/Actions/Proto/ComponentWrapper.hpp"
#include "Solver/Actions/Proto/ConfigurableConstant.hpp"
#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/Expression.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"
#include "Solver/Actions/Proto/Transforms.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Solver/Tags.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::Solver;
using namespace cf3::Solver::Actions;
using namespace cf3::Solver::Actions::Proto;

/// Check close, for testing purposes
inline void check_close(const RealMatrix2& a, const RealMatrix2& b, const Real threshold)
{
  for(Uint i = 0; i != a.rows(); ++i)
    for(Uint j = 0; j != a.cols(); ++j)
      BOOST_CHECK_CLOSE(a(i,j), b(i,j), threshold);
}

static boost::proto::terminal< void(*)(const RealMatrix2&, const RealMatrix2&, Real) >::type const _check_close = {&check_close};

////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( ProtoComponentsSuite )

using boost::proto::lit;

/// Simple test grammar
struct SimpleGrammar :
  boost::proto::or_
  <
    Scalar,
    StreamOutput<SimpleGrammar>,
    boost::proto::otherwise< boost::proto::_default<SimpleGrammar> >
  >
{
};

/// Check_equal function for use inside expressions
inline void check_equal(const Uint a, const Uint b)
{
  BOOST_CHECK_EQUAL(a, b);
}

static boost::proto::terminal< void(*)(const Uint, const Uint) >::type const _check_equal = {&check_equal};


//////////////////////////////////////////////////////////////////////////////

/// Check if copying references works as expected
BOOST_AUTO_TEST_CASE( CopiedReference )
{
  Uint a = 1;
  Uint b = 2;
  Uint& b_ref = b;
  b = 1;
  SimpleGrammar()( DeepCopy()(_check_equal(a, b_ref)) );
}

BOOST_AUTO_TEST_CASE( UseConfigurableConstant )
{
  ConstantStorage constant_values;

  ConfigurableConstant<Real> c("SomeConst", "SomeDescription", 1.);
  Real result;

  // Check if the default value is used
  SimpleGrammar()(ReplaceConfigurableConstants()(lit(result) = c, constant_values));
  BOOST_CHECK_EQUAL(result, 1.);
  BOOST_CHECK_EQUAL(constant_values.m_scalars["SomeConst"], 1.);
  BOOST_CHECK_EQUAL(constant_values.descriptions["SomeConst"], "SomeDescription");
  Real* orig_address = &constant_values.m_scalars["SomeConst"];

  // Mimic a change to the value
  constant_values.m_scalars["SomeConst"] = 2.;
  SimpleGrammar()(ReplaceConfigurableConstants()(lit(result) = c, constant_values));
  BOOST_CHECK_EQUAL(orig_address, &constant_values.m_scalars["SomeConst"]);
  BOOST_CHECK_EQUAL(result, 2.);
  BOOST_CHECK_EQUAL(constant_values.m_scalars["SomeConst"], 2.);
}

/// Set up a model
BOOST_AUTO_TEST_CASE( SetupModel )
{
  CModel& model = Core::instance().root().create_component<CModel>("Model");
  model.create_physics("CF.Physics.DynamicModel");
  CDomain& dom = model.create_domain("Domain");
  CMesh& mesh = dom.create_component<CMesh>("mesh");
  Tools::MeshGeneration::create_line(mesh, 1., 5);
}

/// Test option creation from an expression
BOOST_AUTO_TEST_CASE( ExpressionOptions )
{
  Real result = 0;
  ConfigurableConstant<Real> a("a", "Multiplication factor");

  // Create an expression that sums the volume of a mesh, multiplied with a configurable factor
  boost::shared_ptr<Expression> expression = elements_expression(lit(result) += a*volume);

  CModel& model = Core::instance().root().get_child("Model").as_type<CModel>();

  expression->add_options(model.options());
  BOOST_CHECK(model.options().check("a"));

  // a is zero by default, so the result should be zero
  expression->loop(model.domain().get_child("mesh").as_type<CMesh>().topology());
  BOOST_CHECK_EQUAL(result, 0.);

  // reconfigure a as 1, and check result
  model.options().option("a").change_value(1.);
  expression->loop(model.domain().get_child("mesh").as_type<CMesh>().topology());
  BOOST_CHECK_EQUAL(result, 1.);
}

/// Test variable registration with a physical model
BOOST_AUTO_TEST_CASE( PhysicalModelUsage )
{
  const Uint nb_segments = 5;
  CModel& model = Core::instance().root().get_child("Model").as_type<CModel>();
  CMesh& mesh = model.domain().get_child("mesh").as_type<CMesh>();

  // Declare a mesh variable
  MeshTerm<0, ScalarField> T("Temperature", "solution");

  // Expression to set the temperature field
  boost::shared_ptr<Expression> init_temp = nodes_expression(T = 288.);
  // Register the variables
  init_temp->register_variables(model.physics());

  // Create the fields
  FieldManager& field_manager = model.create_component<FieldManager>("FieldManager");
  field_manager.configure_option("variable_manager", model.physics().variable_manager().uri());
  field_manager.create_field("solution", mesh.geometry());
  BOOST_CHECK(find_component_ptr_with_tag<Field>(mesh.geometry(), "solution"));

  // Do the initialization
  init_temp->loop(model.domain().get_child("mesh").as_type<CMesh>().topology());

  // Sum up the values for the temperature
  Real temp_sum = 0.;
  nodes_expression(lit(temp_sum) += T)->loop(model.domain().get_child("mesh").as_type<CMesh>().topology());
  BOOST_CHECK_EQUAL(temp_sum / static_cast<Real>(1+nb_segments), 288.);
}

/// Test CProtoAction
BOOST_AUTO_TEST_CASE( ProtoAction )
{
  const Uint nb_segments = 5;
  CModel& model = Core::instance().root().get_child("Model").as_type<CModel>();
  CMesh& mesh = model.domain().get_child("mesh").as_type<CMesh>();
  FieldManager& field_manager = model.get_child("FieldManager").as_type<FieldManager>();

  // Declare a mesh variable
  MeshTerm<0, ScalarField> T("Temperature2", "T2");

  // Create an action that can wrap an expression
  CProtoAction& action = Core::instance().root().create_component<CProtoAction>("Action");
  action.set_expression(nodes_expression(T = 288.));
  action.configure_option("physical_model", model.physics().uri());
  action.configure_option(Solver::Tags::regions(), std::vector<URI>(1, model.domain().get_child("mesh").as_type<CMesh>().topology().uri()));

  // Create the fields
  field_manager.create_field("T2", mesh.geometry());
  BOOST_CHECK(find_component_ptr_with_tag<Field>(mesh.geometry(), "T2"));

  // Run the action
  action.execute();

  // Sum up the values for the temperature
  Real temp_sum = 0.;
  nodes_expression(lit(temp_sum) += T)->loop(model.domain().get_child("mesh").as_type<CMesh>().topology());
  BOOST_CHECK_EQUAL(temp_sum / static_cast<Real>(1+nb_segments), 288.);
}

/// Test CSimpleSolver
BOOST_AUTO_TEST_CASE( SimpleSolver )
{
  const Uint nb_segments = 5;
  CModel& model = Core::instance().root().get_child("Model").as_type<CModel>();
  CMesh& mesh = model.domain().get_child("mesh").as_type<CMesh>();

  // Declare a mesh variable
  MeshTerm<0, ScalarField> T("Temperature3", "T3");

  CSolver& solver = model.create_component<CSimpleSolver>("GenericSolver");

  // Storage for the result check
  Real temp_sum = 0.;

  // Add the actions to run on the domain
  solver << create_proto_action( "SetTemp",     nodes_expression(T = 288.) )
         << create_proto_action( "CheckResult", nodes_expression(lit(temp_sum) += T));

  solver.configure_option_recursively(Solver::Tags::regions(), std::vector<URI>(1, model.domain().get_child("mesh").as_type<CMesh>().topology().uri()));
  solver.configure_option_recursively("physical_model", model.physics().uri());
  solver.mesh_loaded(model.domain().get_child("mesh").as_type<CMesh>());

  solver.field_manager().create_field("T3", mesh.geometry());

  // Run the actions
  model.simulate();

  // Check result
  BOOST_CHECK_EQUAL(temp_sum / static_cast<Real>(1+nb_segments), 288.);
}

/// Create a custom actiondomain
class CustomProtoSolver : public CSimpleSolver
{
public:

  typedef boost::shared_ptr<CustomProtoSolver> Ptr;
  typedef boost::shared_ptr<CustomProtoSolver const> ConstPtr;

  CustomProtoSolver(const std::string& name) :
    CSimpleSolver(name),
    T("Temperature4", "T4"),
    temp_sum(0.)
  {
    // Add the expressions
    *this << create_proto_action( "SetTemp",  nodes_expression(T = 288.) )
          << create_proto_action( "Output", nodes_expression(_cout << T << "\n"))
          << create_proto_action( "SumTemps", nodes_expression(lit(temp_sum) += T));
  }

  virtual void mesh_loaded(CMesh& mesh)
  {
    field_manager().create_field("T4", mesh.geometry());
  }

  MeshTerm<0, ScalarField> T;
  Real temp_sum;

  static std::string type_name() { return "CustomProtoSolver"; }
};

BOOST_AUTO_TEST_CASE( ProtoCustomSolver )
{
  const Uint nb_segments = 5;
  CModel& model = Core::instance().root().get_child("Model").as_type<CModel>();

  CustomProtoSolver& solver = model.create_component<CustomProtoSolver>("CustomSolver");
  solver.configure_option_recursively(Solver::Tags::regions(), std::vector<URI>(1, model.domain().get_child("mesh").as_type<CMesh>().topology().uri()));
  solver.configure_option_recursively(Solver::Tags::physical_model(), model.physics().uri());
  solver.mesh_loaded(model.domain().get_child("mesh").as_type<CMesh>());

  // Run the actions
  model.simulate();
  print_timing_tree(model);

  // Check
  BOOST_CHECK_EQUAL(solver.temp_sum / static_cast<Real>(1+nb_segments), 288.);
}

// Check if references inside terminals are kept corectly
BOOST_AUTO_TEST_CASE( CopyExpression )
{
  const Uint nb_segments = 5;
  CMesh& mesh = Core::instance().root().get_child("Model").as_type<CModel>().domain().get_child("mesh").as_type<CMesh>();

  Real a = 1.;
  Real result = 0.;

  // Create an expression that sums the volume of a mesh, multiplied with a configurable factor
  boost::shared_ptr<Expression> expression = elements_expression(lit(result) += a*volume + 2. * boost::proto::lit(1.));

  expression->loop(mesh.topology());

  // default result
  BOOST_CHECK_EQUAL(result, 11.);

  // reconfigure a as 2, and check result
  a = 2.;
  result = 0;
  expression->loop(mesh.topology());
  BOOST_CHECK_EQUAL(result, 12.);
}

// Primitive transform that prints a component URI when given a ComponentWrapper terminal
struct ComponentURIPrinter :
  boost::proto::transform< ComponentURIPrinter >
{
  template<typename ExprT, typename StateT, typename DataT>
  struct impl : boost::proto::transform_impl<ExprT, StateT, DataT>
  {
    typedef void result_type;

    result_type operator ()(typename impl::expr_param expr, typename impl::state_param state, typename impl::data_param data) const
    {
      std::cout << boost::proto::value(boost::proto::left(expr)).component().uri().string();
    }
  };
};

/// Define a tag (not used here)
struct SomeTag {};

BOOST_AUTO_TEST_CASE( ComponentWrapperURI )
{
  CModel& model = Core::instance().root().get_child("Model").as_type<CModel>();

  BOOST_CHECK_EQUAL(model.physics().uri().string(), "cpath://Root/Model/DynamicModel");

  ComponentWrapper<Physics::PhysModel, SomeTag> wrapped_phys_model(model.get_child("CustomSolver").option(Solver::Tags::physical_model()));

  BOOST_CHECK_EQUAL(boost::proto::value(wrapped_phys_model).component().uri().string(), "cpath://Root/Model/DynamicModel");

  ComponentURIPrinter()(DeepCopy()(wrapped_phys_model + 1));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
