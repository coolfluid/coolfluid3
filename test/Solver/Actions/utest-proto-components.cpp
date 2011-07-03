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

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/CMesh.hpp"

#include "Solver/CPhysicalModel.hpp"

#include "Solver/Actions/Proto/ConfigurableConstant.hpp"
#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/CProtoActionDirector.hpp"
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

/// Check if values stored in a StoredReference change as expected
BOOST_AUTO_TEST_CASE( StoredReferenceValue )
{
  Uint a = 1;
  Uint b = 2;
  StoredReference<Uint> b_ref(b);
  b = 1;
  SimpleGrammar()(_check_equal(a, b_ref));
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

/// Test option creation from an expression
BOOST_AUTO_TEST_CASE( ExpressionOptions )
{
  Real result = 0;
  ConfigurableConstant<Real> a("a", "Multiplication factor");
 
  // Create an expression that sums the volume of a mesh, multiplied with a configurable factor
  boost::shared_ptr<Expression> expression = elements_expression(lit(store(result)) += a*volume);
  
  // Test mesh
  CMesh& mesh = Core::instance().root().create_component<CMesh>("line");
  Tools::MeshGeneration::create_line(mesh, 1., 100);
  
  expression->add_options(mesh.options());
  BOOST_CHECK(mesh.options().check("a"));
  
  // a is zero by default, so the result should be zero
  expression->loop(mesh.topology());
  BOOST_CHECK_EQUAL(result, 0.);
  
  // reconfigure a as 1, and check result
  mesh.options().option("a").change_value(1.);
  expression->loop(mesh.topology());
  BOOST_CHECK_EQUAL(result, 1.);
}

/// Test variable registration with a physical model
BOOST_AUTO_TEST_CASE( PhysicalModelUsage )
{ 
  const Uint nb_segments = 100;
  
  // Component tree setup
  CMesh& mesh = Core::instance().root().create_component<CMesh>("line2");
  Tools::MeshGeneration::create_line(mesh, 1., nb_segments);
  
  CPhysicalModel& physical_model = Core::instance().root().create_component<CPhysicalModel>("PhysicalModel");
  physical_model.configure_option("mesh", mesh.uri());
  
  // Declare a mesh variable
  MeshTerm<0, ScalarField> T("Temperature", "T");
  
  // Expression to set the temperature field
  boost::shared_ptr<Expression> init_temp = nodes_expression(T = 288.);
  // Register the variables
  init_temp->register_variables(physical_model);
  
  // Create the fields
  physical_model.create_fields();
  BOOST_CHECK(mesh.get_child_ptr("Temperature"));
  
  // Do the initialization
  init_temp->loop(mesh.topology());
  
  // Sum up the values for the temperature
  Real temp_sum = 0.;
  nodes_expression(lit(store(temp_sum)) += T)->loop(mesh.topology());
  BOOST_CHECK_EQUAL(temp_sum / static_cast<Real>(1+nb_segments), 288.);
}

/// Test CProtoAction
BOOST_AUTO_TEST_CASE( ProtoAction )
{ 
  const Uint nb_segments = 100;
  
  // Component tree setup
  CMesh& mesh = Core::instance().root().create_component<CMesh>("line3");
  Tools::MeshGeneration::create_line(mesh, 1., nb_segments);
  
  CPhysicalModel& physical_model = Core::instance().root().create_component<CPhysicalModel>("PhysicalModel2");
  physical_model.configure_option("mesh", mesh.uri());
  
  // Declare a mesh variable
  MeshTerm<0, ScalarField> T("Temperature", "T");
  
  // Create an action that can wrap an expression
  CProtoAction& action = Core::instance().root().create_component<CProtoAction>("Action");
  action.set_expression(nodes_expression(T = 288.));
  action.configure_option("physical_model", physical_model.uri());
  action.configure_option("region", mesh.topology().uri());
  
  // Create the fields
  physical_model.create_fields();
  BOOST_CHECK(mesh.get_child_ptr("Temperature"));
  
  // Run the action
  action.execute();
  
  // Sum up the values for the temperature
  Real temp_sum = 0.;
  nodes_expression(lit(store(temp_sum)) += T)->loop(mesh.topology());
  BOOST_CHECK_EQUAL(temp_sum / static_cast<Real>(1+nb_segments), 288.);
}

/// Test CProtoActionDomain
BOOST_AUTO_TEST_CASE( ProtoActionDomain )
{ 
  const Uint nb_segments = 100;
  
  // Create a domain
  CProtoActionDirector& domain = Core::instance().root().create_component<CProtoActionDirector>("domain");
  CMesh& mesh = domain.create_component<CMesh>("mesh");
  CPhysicalModel& physical_model = domain.create_component<CPhysicalModel>("physical_model");
  domain.configure_option("physical_model", physical_model.uri());
  
  // Setup mesh
  Tools::MeshGeneration::create_line(mesh, 1., nb_segments);
  
  // Declare a mesh variable
  MeshTerm<0, ScalarField> T("Temperature", "T");
  
  // Storage for the result check
  Real temp_sum = 0.;
  
  // Add the actions to run on the domain
  domain << domain.add_action( "SetTemp",     nodes_expression(T = 288.) )
         << domain.add_action( "CheckResult", nodes_expression(lit(store(temp_sum)) += T));
         
  domain.configure_option("region", mesh.topology().uri());
  
  // Complete setup
  physical_model.configure_option("mesh", mesh.uri());
         
  // Run the actions
  domain.execute();
  
  // Check result
  BOOST_CHECK_EQUAL(temp_sum / static_cast<Real>(1+nb_segments), 288.);
}

/// Create a custom actiondomain
class CustomProtoSolver : public CProtoActionDirector
{
public:
  
  typedef boost::shared_ptr<CustomProtoSolver> Ptr;
  typedef boost::shared_ptr<CustomProtoSolver const> ConstPtr;
  
  CustomProtoSolver(const std::string& name) :
    CProtoActionDirector(name),
    temp_sum(0.)
  {
    // Term to use
    MeshTerm<0, ScalarField> T("Temperature", "T");
    
    // Add the expressions
    *this << add_action( "SetTemp",  nodes_expression(T = 288.) )
          << add_action( "SumTemps", nodes_expression(lit(store(temp_sum)) += T));
  }
  
  Real temp_sum;
  
  static std::string type_name() { return "CustomProtoSolver"; }
};



BOOST_AUTO_TEST_CASE( ProtoCustomSolver )
{ 
  const Uint nb_segments = 100;
  
  // Create a domain
  CustomProtoSolver& domain = Core::instance().root().create_component<CustomProtoSolver>("custom_domain");
  CMesh& mesh = domain.create_component<CMesh>("mesh");
  CPhysicalModel& physical_model = domain.create_component<CPhysicalModel>("physical_model");
  domain.configure_option("physical_model", physical_model.uri());
  
  // Setup mesh
  Tools::MeshGeneration::create_line(mesh, 1., nb_segments);
  physical_model.configure_option("mesh", mesh.uri());
  domain.configure_option("region", mesh.topology().uri());
  
  // Run the actions
  domain.execute();
  
  // Check
  BOOST_CHECK_EQUAL(domain.temp_sum / static_cast<Real>(1+nb_segments), 288.);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
