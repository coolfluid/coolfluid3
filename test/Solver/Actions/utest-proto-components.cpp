// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for proto operations related to components"

#include <map>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/CActionDirector.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CModel.hpp"
#include "Solver/CreateFields.hpp"
#include "Solver/CSimpleSolver.hpp"

#include "Solver/Actions/Proto/ConfigurableConstant.hpp"
#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/Expression.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"
#include "Solver/Actions/Proto/Transforms.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;

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
  
  // Declare a mesh variable
  MeshTerm<0, ScalarField> T("Temperature", "T");
  
  // Expression to set the temperature field
  boost::shared_ptr<Expression> init_temp = nodes_expression(T = 288.);
  // Register the variables
  init_temp->register_variables(model.physics());
  
  // Create the fields
  create_fields(model.domain().get_child("mesh").as_type<CMesh>(), model.physics());
  BOOST_CHECK(model.domain().get_child("mesh").as_type<CMesh>().get_child_ptr("Temperature"));
  
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
  
  // Declare a mesh variable
  MeshTerm<0, ScalarField> T("Temperature2", "T2");
  
  // Create an action that can wrap an expression
  CProtoAction& action = Core::instance().root().create_component<CProtoAction>("Action");
  action.set_expression(nodes_expression(T = 288.));
  action.configure_option("physical_model", model.physics().uri());
  action.configure_option("regions", std::vector<URI>(1, model.domain().get_child("mesh").as_type<CMesh>().topology().uri()));
  
  // Create the fields
  create_fields(model.domain().get_child("mesh").as_type<CMesh>(), model.physics());
  BOOST_CHECK(model.domain().get_child("mesh").as_type<CMesh>().get_child_ptr("Temperature2"));
  
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
  
  // Declare a mesh variable
  MeshTerm<0, ScalarField> T("Temperature3", "T3");
  
  CSolver& solver = model.create_component<CSimpleSolver>("GenericSolver");
  
  // Storage for the result check
  Real temp_sum = 0.;
  
  // Add the actions to run on the domain
  solver << create_proto_action( "SetTemp",     nodes_expression(T = 288.) )
         << create_proto_action( "CheckResult", nodes_expression(lit(temp_sum) += T));
  
  solver.configure_option_recursively("regions", std::vector<URI>(1, model.domain().get_child("mesh").as_type<CMesh>().topology().uri()));
  solver.configure_option_recursively("physical_model", model.physics().uri());
  solver.mesh_loaded(model.domain().get_child("mesh").as_type<CMesh>());
  
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
  
  MeshTerm<0, ScalarField> T;
  Real temp_sum;
  
  static std::string type_name() { return "CustomProtoSolver"; }
};

BOOST_AUTO_TEST_CASE( ProtoCustomSolver )
{ 
  const Uint nb_segments = 5;
  CModel& model = Core::instance().root().get_child("Model").as_type<CModel>();
  
  CustomProtoSolver& solver = model.create_component<CustomProtoSolver>("CustomSolver");
  solver.configure_option_recursively("regions", std::vector<URI>(1, model.domain().get_child("mesh").as_type<CMesh>().topology().uri()));
  solver.configure_option_recursively("physical_model", model.physics().uri());
  solver.mesh_loaded(model.domain().get_child("mesh").as_type<CMesh>());
  
  // Run the actions
  model.simulate();
  
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

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
