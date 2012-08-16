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

#include "common/ActionDirector.hpp"
#include "common/Core.hpp"
#include "common/Log.hpp"
#include "common/TimedComponent.hpp"
#include <common/Environment.hpp>

#include "math/MatrixTypes.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"

#include "physics/PhysModel.hpp"

#include "solver/Model.hpp"
#include "solver/SimpleSolver.hpp"

#include "solver/actions/Proto/ComponentWrapper.hpp"
#include "solver/actions/Proto/ConfigurableConstant.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/Terminals.hpp"
#include "solver/actions/Proto/Transforms.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "solver/Tags.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;

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
  Core::instance().environment().options().set("log_level", 4);
  
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
  Model& model = *Core::instance().root().create_component<Model>("Model");
  model.create_physics("cf3.physics.DynamicModel");
  Domain& dom = model.create_domain("Domain");
  Mesh& mesh = *dom.create_component<Mesh>("mesh");
  Tools::MeshGeneration::create_line(mesh, 1., 5);
}

/// Test option creation from an expression
BOOST_AUTO_TEST_CASE( ExpressionOptions )
{
  Real result = 0;
  RealVector default_a(3);
  default_a.setZero();;
  ConfigurableConstant<RealVector> a("a", "Multiplication factor", default_a);

  // Create an expression that sums the volume of a mesh, multiplied with a configurable factor
  boost::shared_ptr<Expression> expression = elements_expression(lit(result) += boost::proto::lit(a)[0]*volume);

  Handle<Model> model(Core::instance().root().get_child("Model"));

  expression->add_options(model->options());
  BOOST_CHECK(model->options().check("a"));

  // a is zero by default, so the result should be zero
  expression->loop(model->domain().get_child("mesh")->handle<Mesh>()->topology());
  BOOST_CHECK_EQUAL(result, 0.);

  // reconfigure a as 1, and check result
  model->options().option("a").change_value(std::vector<Real>(1, 1.));
  expression->loop(model->domain().get_child("mesh")->handle<Mesh>()->topology());
  BOOST_CHECK_EQUAL(result, 1.);
}

/// Test variable registration with a physical model
BOOST_AUTO_TEST_CASE( PhysicalModelUsage )
{
  const Uint nb_segments = 5;
  Handle<Model> model(Core::instance().root().get_child("Model"));
  Handle<Mesh> mesh(model->domain().get_child("mesh"));

  // Declare a mesh variable
  FieldVariable<0, ScalarField> T("Temperature", "solution");

  // Expression to set the temperature field
  boost::shared_ptr<Expression> init_temp = nodes_expression(T = 288.);
  // Register the variables
  init_temp->register_variables(model->physics());

  // Create the fields
  FieldManager& field_manager = *model->create_component<FieldManager>("FieldManager");
  field_manager.options().set("variable_manager", model->physics().variable_manager().handle<math::VariableManager>());
  field_manager.create_field("solution", mesh->geometry_fields());
  BOOST_CHECK(find_component_ptr_with_tag<Field>(mesh->geometry_fields(), "solution"));

  // Do the initialization
  init_temp->loop(model->domain().get_child("mesh")->handle<Mesh>()->topology());

  // Sum up the values for the temperature
  Real temp_sum = 0.;
  nodes_expression(lit(temp_sum) += T)->loop(model->domain().get_child("mesh")->handle<Mesh>()->topology());
  BOOST_CHECK_EQUAL(temp_sum / static_cast<Real>(1+nb_segments), 288.);
}

/// Test ProtoAction
BOOST_AUTO_TEST_CASE( ProtoActionTest )
{
  const Uint nb_segments = 5;
  Handle<Model> model(Core::instance().root().get_child("Model"));
  Handle<Mesh> mesh(model->domain().get_child("mesh"));
  Handle<FieldManager> field_manager(model->get_child("FieldManager"));

  // Declare a mesh variable
  FieldVariable<0, ScalarField> T("Temperature2", "T2");

  // Create an action that can wrap an expression
  ProtoAction& action = *Core::instance().root().create_component<ProtoAction>("Action");
  action.set_expression(nodes_expression(T = 288.));
  action.options().set("physical_model", model->physics().handle<physics::PhysModel>());
  action.options().set(solver::Tags::regions(), std::vector<URI>(1, model->domain().get_child("mesh")->handle<Mesh>()->topology().uri()));

  // Create the fields
  field_manager->create_field("T2", mesh->geometry_fields());
  BOOST_CHECK(find_component_ptr_with_tag<Field>(mesh->geometry_fields(), "T2"));

  // Run the action
  action.execute();

  // Sum up the values for the temperature
  Real temp_sum = 0.;
  nodes_expression(lit(temp_sum) += T)->loop(model->domain().get_child("mesh")->handle<Mesh>()->topology());
  BOOST_CHECK_EQUAL(temp_sum / static_cast<Real>(1+nb_segments), 288.);
}

/// Test SimpleSolver
BOOST_AUTO_TEST_CASE( SimpleSolverTest )
{
  const Uint nb_segments = 5;
  Handle<Model> model(Core::instance().root().get_child("Model"));
  Handle<Mesh> mesh(model->domain().get_child("mesh"));

  // Declare a mesh variable
  FieldVariable<0, ScalarField> T("Temperature3", "T3");

  Solver& solver = *model->create_component<SimpleSolver>("GenericSolver");

  // Storage for the result check
  Real temp_sum = 0.;

  // Add the actions to run on the domain
  solver << create_proto_action( "SetTemp",     nodes_expression(T = 288.) )
         << create_proto_action( "CheckResult", nodes_expression(lit(temp_sum) += T));

  solver.configure_option_recursively(solver::Tags::regions(), std::vector<URI>(1, mesh->topology().uri()));
  solver.configure_option_recursively("physical_model", model->physics().handle<physics::PhysModel>());
  solver.mesh_loaded(*mesh);

  solver.field_manager().create_field("T3", mesh->geometry_fields());

  // Run the actions
  model->simulate();

  // Check result
  BOOST_CHECK_EQUAL(temp_sum / static_cast<Real>(1+nb_segments), 288.);
  
  model->remove_component("GenericSolver");
}

/// Test Physics constants
BOOST_AUTO_TEST_CASE( PhysicsSolverTest )
{
  const Uint nb_segments = 5;
  Handle<Model> model(Core::instance().root().get_child("Model"));
  Handle<Mesh> mesh(model->domain().get_child("mesh"));

  model->physics().options().add("input_temperature", 288.);
  
  // Declare a mesh variable
  FieldVariable<0, ScalarField> T("TemperaturePhysics", "TP");
  PhysicsConstant Tin("input_temperature");

  Solver& solver = *model->create_component<SimpleSolver>("PhysicsSolver");

  // Storage for the result check
  Real temp_sum = 0.;

  // Add the actions to run on the domain
  solver << create_proto_action( "SetTemp",     nodes_expression(T = Tin) )
         << create_proto_action( "CheckResult", nodes_expression(lit(temp_sum) += T));

  solver.configure_option_recursively(solver::Tags::regions(), std::vector<URI>(1, mesh->topology().uri()));
  solver.configure_option_recursively("physical_model", model->physics().handle<physics::PhysModel>());
  solver.mesh_loaded(*mesh);

  solver.field_manager().create_field("TP", mesh->geometry_fields());
  
  // Run the actions
  model->simulate();

  // Check result
  BOOST_CHECK_EQUAL(temp_sum / static_cast<Real>(1+nb_segments), 288.);
  
  // Change value and re-run
  temp_sum = 0;
  model->physics().options().set("input_temperature", 100.);
  model->simulate();
  BOOST_CHECK_EQUAL(temp_sum / static_cast<Real>(1+nb_segments), 100.);
  
  
  model->remove_component("PhysicsSolver");
}

/// Create a custom actiondomain
class CustomProtoSolver : public SimpleSolver
{
public:
  CustomProtoSolver(const std::string& name) :
    SimpleSolver(name),
    T("Temperature4", "T4"),
    temp_sum(0.)
  {
    // Add the expressions
    *this << create_proto_action( "SetTemp",  nodes_expression(T = 288.) )
          << create_proto_action( "Output", nodes_expression(_cout << T << "\n"))
          << create_proto_action( "SumTemps", nodes_expression(lit(temp_sum) += T));
  }

  virtual void mesh_loaded(Mesh& mesh)
  {
    field_manager().create_field("T4", mesh.geometry_fields());
  }

  FieldVariable<0, ScalarField> T;
  Real temp_sum;

  static std::string type_name() { return "CustomProtoSolver"; }
};

BOOST_AUTO_TEST_CASE( ProtoCustomSolver )
{
  const Uint nb_segments = 5;
  Handle<Model> model(Core::instance().root().get_child("Model"));

  Handle<CustomProtoSolver> solver = model->create_component<CustomProtoSolver>("CustomSolver");
  solver->configure_option_recursively(solver::Tags::regions(), std::vector<URI>(1, model->domain().get_child("mesh")->handle<Mesh>()->topology().uri()));
  solver->configure_option_recursively(solver::Tags::physical_model(), model->physics().handle<physics::PhysModel>());
  solver->mesh_loaded(*model->domain().get_child("mesh")->handle<Mesh>());

  // Run the actions
  model->simulate();
  print_timing_tree(*model);

  // Check
  BOOST_CHECK_EQUAL(solver->temp_sum / static_cast<Real>(1+nb_segments), 288.);
}

// Check if references inside terminals are kept corectly
BOOST_AUTO_TEST_CASE( CopyExpression )
{
  const Uint nb_segments = 5;
  Handle<Mesh> mesh(Core::instance().root().get_child("Model")->handle<Model>()->domain().get_child("mesh"));

  Real a = 1.;
  Real result = 0.;

  // Create an expression that sums the volume of a mesh, multiplied with a configurable factor
  boost::shared_ptr<Expression> expression = elements_expression(lit(result) += a*volume + 2. * boost::proto::lit(1.));

  expression->loop(mesh->topology());

  // default result
  BOOST_CHECK_EQUAL(result, 11.);

  // reconfigure a as 2, and check result
  a = 2.;
  result = 0;
  expression->loop(mesh->topology());
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
  Handle<Model> model(Core::instance().root().get_child("Model"));

  BOOST_CHECK_EQUAL(model->physics().uri().string(), "cpath:/Model/DynamicModel");

  ComponentWrapper<physics::PhysModel, SomeTag> wrapped_phys_model(model->get_child("CustomSolver")->options().option(solver::Tags::physical_model()));

  BOOST_CHECK_EQUAL(boost::proto::value(wrapped_phys_model).component().uri().string(), "cpath:/Model/DynamicModel");

  ComponentURIPrinter()(DeepCopy()(wrapped_phys_model + 1));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
