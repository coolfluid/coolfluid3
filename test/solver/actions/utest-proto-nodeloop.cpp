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
#include <solver/actions/Proto/ProtoAction.hpp>

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

#include "solver/Tags.hpp"

#include "physics/PhysModel.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "Tools/Testing/TimedTestFixture.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::mesh;
using namespace cf3::common;

using namespace cf3::math::Consts;

////////////////////////////////////////////////////


BOOST_FIXTURE_TEST_SUITE( ProtoOperatorsSuite, Tools::Testing::TimedTestFixture )

using boost::proto::lit;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( InitFields )
{
  Model& model = *Core::instance().root().create_component<Model>("Model");
  model.create_physics("cf3.physics.DynamicModel");
  Domain& dom = model.create_domain("Domain");
  Mesh& mesh = *dom.create_component<Mesh>("mesh");
  Tools::MeshGeneration::create_rectangle(mesh, 1., 1., 500, 500);
  
  FieldManager& field_manager = *model.create_component<FieldManager>("FieldManager");
  field_manager.options().set("variable_manager", model.physics().variable_manager().handle<math::VariableManager>());

  FieldVariable<0, VectorField> u("u","velocity");
  FieldVariable<2, VectorField> u_adv("u_adv", "advection");
  FieldVariable<3, VectorField> u1("u1", "advection");
  FieldVariable<4, VectorField> u2("u2", "advection");
  FieldVariable<5, VectorField> u3("u3", "advection");
  
  // Create an action that can wrap an expression
  ProtoAction& action = *model.create_component<ProtoAction>("ActionInit");
  action.set_expression(nodes_expression(group(u_adv[_i] = 0., u[_i] = 1., u1[_i] = 1., u2[_i] = 1., u3[_i] = 1.)));
  action.options().set("physical_model", model.physics().handle<physics::PhysModel>());
  action.options().set(solver::Tags::regions(), std::vector<URI>(1, model.domain().get_child("mesh")->handle<Mesh>()->topology().uri()));

  // Create the fields
  field_manager.create_field("velocity", mesh.geometry_fields());
  field_manager.create_field("advection", mesh.geometry_fields());
  
  action.execute();
}

BOOST_AUTO_TEST_CASE( LinearizeU )
{
  Handle<Model> model(Core::instance().root().get_child("Model"));

  FieldVariable<0, VectorField> u("u","velocity");
  FieldVariable<2, VectorField> u_adv("u_adv", "advection");
  FieldVariable<3, VectorField> u1("u1", "advection");
  FieldVariable<4, VectorField> u2("u2", "advection");
  FieldVariable<5, VectorField> u3("u3", "advection");
  
  // Create an action that can wrap an expression
  ProtoAction& action = *model->create_component<ProtoAction>("ActionU");
  action.set_expression(nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3));
  action.options().set("physical_model", model->physics().handle<physics::PhysModel>());
  action.options().set(solver::Tags::regions(), std::vector<URI>(1, model->domain().get_child("mesh")->handle<Mesh>()->topology().uri()));
  
  action.execute();
}

BOOST_AUTO_TEST_CASE( CheckResult )
{
  Handle<Model> model(Core::instance().root().get_child("Model"));

  RealVector result(2); result.setZero();
  FieldVariable<0, VectorField> u_adv("u_adv", "advection");
  
  // Create an action that can wrap an expression
  ProtoAction& action = *model->create_component<ProtoAction>("ActionCheck");
  action.set_expression(nodes_expression(lit(result) += u_adv));
  action.options().set("physical_model", model->physics().handle<physics::PhysModel>());
  action.options().set(solver::Tags::regions(), std::vector<URI>(1, model->domain().get_child("mesh")->handle<Mesh>()->topology().uri()));
  
  action.execute();
  
  Handle<Mesh> mesh(model->domain().get_child("mesh"));
  result /= mesh->geometry_fields().size();
  
  BOOST_CHECK_CLOSE(result[0], 1., 1e-8);
  BOOST_CHECK_CLOSE(result[1], 1., 1e-8);
}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
