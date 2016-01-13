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

#include <Thyra_describeLinearOp.hpp>
#include <Thyra_MultiVectorBase.hpp>
#include <Thyra_MultiVectorStdOps.hpp>
#include <Thyra_VectorBase.hpp>
#include <Thyra_VectorSpaceBase.hpp>
#include <Thyra_VectorStdOps.hpp>
#include <Teuchos_VerboseObject.hpp>

#include "solver/Model.hpp"
#include "solver/Solver.hpp"

#include "solver/actions/Proto/ElementLooper.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/Functions.hpp"
#include "solver/actions/Proto/NodeLooper.hpp"
#include "solver/actions/Proto/Terminals.hpp"

#include "common/Core.hpp"
#include "common/Log.hpp"
#include "common/Environment.hpp"
#include "common/FindComponents.hpp"

#include "math/MatrixTypes.hpp"
#include "math/LSS/System.hpp"
#include "math/LSS/Vector.hpp"
#include "math/LSS/Trilinos/ThyraOperator.hpp"
#include "math/LSS/Trilinos/ThyraVector.hpp"
#include <math/LSS/SolveLSS.hpp>

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Entities.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Dictionary.hpp"

#include "mesh/Integrators/Gauss.hpp"
#include "mesh/ElementTypes.hpp"

#include "physics/PhysModel.hpp"

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

struct ProtoLSSFixture
{
  ProtoLSSFixture() :
    root(Core::instance().root())
  {
    if(is_null(model))
    {
      common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);

      common::Core::instance().environment().options().set("log_level", 3);
      model = Core::instance().root().create_component<Model>("Model");
      physical_model = Handle<physics::PhysModel>(model->create_physics("cf3.physics.DynamicModel").handle());
      Domain& dom = model->create_domain("Domain");
      mesh = dom.create_component<Mesh>("mesh");
      //Tools::MeshGeneration::create_rectangle(*mesh, 5., 5., 5, 5);
      //Tools::MeshGeneration::create_line(*mesh, 3.,3);
      Tools::MeshGeneration::create_rectangle_tris(*mesh, 1., 1., 1, 1);

      field_manager = model->create_component<FieldManager>("FieldManager");
      field_manager->options().set("variable_manager", model->physics().variable_manager().handle<math::VariableManager>());

      // Build node connectivity
      const Uint nb_nodes = mesh->geometry_fields().size();
      std::vector< std::set<Uint> > connectivity_sets(nb_nodes);
      BOOST_FOREACH(const Entities& elements, common::find_components_recursively_with_filter<Entities>(*mesh, IsElementsVolume()))
      {
        const Connectivity& connectivity = elements.geometry_space().connectivity();
        const Uint nb_elems = connectivity.size();
        const Uint nb_elem_nodes = connectivity.row_size();
        for(Uint elem = 0; elem != nb_elems; ++elem)
        {
					std::cout << "Connectivity for element " << elem << ":";
          BOOST_FOREACH(const Uint node_a, connectivity[elem])
          {
						std::cout << " " << node_a;
            BOOST_FOREACH(const Uint node_b, connectivity[elem])
            {
              connectivity_sets[node_a].insert(node_b);
            }
          }
					std::cout << std::endl;
        }
      }

			const mesh::Field& coords = mesh->geometry_fields().coordinates();
			for(Uint i = 0; i != nb_nodes; ++i)
			{
				std::cout << "node " << i << ": " << coords[i][0] << ", " << coords[i][1] << std::endl;
			}

      starting_indices.push_back(0);
      BOOST_FOREACH(const std::set<Uint>& nodes, connectivity_sets)
      {
        starting_indices.push_back(starting_indices.back() + nodes.size());
        node_connectivity.insert(node_connectivity.end(), nodes.begin(), nodes.end());
      }

      loop_regions.push_back(mesh->topology().get_child("left")->uri());
    }


  }

  Component& root;
  static Handle<Model> model;
  static Handle<physics::PhysModel> physical_model;
  static Handle<Mesh> mesh;
  static Handle<FieldManager> field_manager;
  static std::vector<URI> loop_regions;

  static std::vector<Uint> node_connectivity;
  static std::vector<Uint> starting_indices;
};

Handle<Model> ProtoLSSFixture::model;
Handle<physics::PhysModel> ProtoLSSFixture::physical_model;
Handle<Mesh> ProtoLSSFixture::mesh;
Handle<FieldManager> ProtoLSSFixture::field_manager;
std::vector<URI> ProtoLSSFixture::loop_regions;

std::vector<Uint> ProtoLSSFixture::node_connectivity;
std::vector<Uint> ProtoLSSFixture::starting_indices;

BOOST_FIXTURE_TEST_SUITE( ProtoLSSSuite, ProtoLSSFixture )

using boost::proto::lit;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( SetRHSTest )
{
  Handle<math::LSS::System> lss = root.create_component<math::LSS::System>("rhs_lss");
  lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  lss->create(mesh->geometry_fields().comm_pattern(), 1, node_connectivity, starting_indices);

  lss->rhs()->reset();

  Handle<ProtoAction> action = root.create_component<ProtoAction>("ScalarLSSAction");

  // Terminals to use
  FieldVariable<0, ScalarField> T("ScalarVar", "scalar");
  SystemMatrix matrix(*lss);
  SystemRHS sys_rhs(*lss);
  SolutionVector sol_vec(*lss);

  // Run the expression
  action->set_expression(nodes_expression(
    group
    (
      sys_rhs(T) = lit(1.)
    )
  ));

  action->options().set("physical_model", physical_model);
  action->options().set(solver::Tags::regions(), loop_regions);

  field_manager->create_field("scalar", mesh->geometry_fields());

  action->execute();

  for(Uint i = 0; i != 4; ++i)
  {
    Real result = 0;
    lss->rhs()->get_value(i, result);
    std::cout << "i: " << i << ", result: " << result << std::endl;
    if(i == 0 || i == 2)
    {
      BOOST_CHECK_EQUAL(result, 1);
    }
    else
    {
      BOOST_CHECK_EQUAL(result, 0);
    }
  }
}

BOOST_AUTO_TEST_CASE( ZeroRowTest )
{
  Handle<math::LSS::System> lss = root.create_component<math::LSS::System>("zero_lss");
  lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  lss->create(mesh->geometry_fields().comm_pattern(), 3, node_connectivity, starting_indices);

  lss->rhs()->reset();



  // Terminals to use
  FieldVariable<0, ScalarField> T("ScalarVar", "scalar");
  FieldVariable<1, VectorField> u("VectorVar", "scalar");
  SystemMatrix matrix(*lss);
  SystemRHS sys_rhs(*lss);
  SolutionVector sol_vec(*lss);

  Handle<ProtoAction> mat_action = root.create_component<ProtoAction>("MassMatrixAction");
  mat_action->set_expression(elements_expression(
    group
    (
      _A = _0, _a = _0,
      element_quadrature
      (
        _A(T,T) += transpose(nabla(T)) * nabla(T),
        _A(u[_i],u[_i]) += transpose(nabla(T)) * nabla(T),
        _a[T] += transpose(N(T))
      ),
      matrix += _A,
      sys_rhs += _a
    )
  ));

  mat_action->options().set("physical_model", physical_model);
  mat_action->options().set(solver::Tags::regions(), std::vector<common::URI>(1, mesh->topology().uri()));

  field_manager->create_field("scalar", mesh->geometry_fields());
  field_manager->create_field("vector[v]", mesh->geometry_fields());

  mat_action->execute();

  RealVector u_init(2);
  u_init[0] = 1.;
  u_init[1] = 2.;

  Handle<ProtoAction> zero_action = root.create_component<ProtoAction>("ZeroRowAction");
  zero_action->set_expression(nodes_expression(
    group
    (
      zero_row(matrix, T),
      u = u_init
    )
  ));

  zero_action->options().set("physical_model", physical_model);
  zero_action->options().set(solver::Tags::regions(), loop_regions);
  zero_action->execute();

  lss->print(std::cout);

  for(Uint i = 0; i != 4; ++i)
  {
    Real result = 0;
    lss->rhs()->get_value(i, 0, result);
    std::vector<Real> diag(4);
    lss->matrix()->get_diagonal(diag);
    if(i == 0 || i == 2)
    {
      BOOST_CHECK_EQUAL(result, 0);
      BOOST_CHECK_EQUAL(diag[i*3], 0);
    }
    else
    {
      BOOST_CHECK(result != 0);
      BOOST_CHECK_EQUAL(diag[i*3], 1);
    }
  }

  // Replace the zeroed row with a no-penetration condition on the boundary
  Handle<ProtoAction> no_penetration = root.create_component<ProtoAction>("NoPenetration");
  no_penetration->set_expression(nodes_expression
  (
    matrix(T,u) = transpose(u)
  ));

  no_penetration->options().set("physical_model", physical_model);
  no_penetration->options().set(solver::Tags::regions(), loop_regions);
  no_penetration->execute();

  lss->matrix()->print_native(std::cout);
  lss->rhs()->print_native(std::cout);
}

BOOST_AUTO_TEST_CASE( WeightTest )
{
  Handle<math::LSS::System> lss = root.create_component<math::LSS::System>("weights_lss");
  lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  lss->create(mesh->geometry_fields().comm_pattern(), 1, node_connectivity, starting_indices);
  lss->rhs()->reset();

  // Terminals to use
  FieldVariable<0, ScalarField> T("ScalarVar", "scalar");
  SystemMatrix matrix(*lss);
  SystemRHS sys_rhs(*lss);
  SolutionVector sol_vec(*lss);

  Handle<ProtoAction> init_scalar = root.create_component<ProtoAction>("InitScalar");
  init_scalar->set_expression(nodes_expression(T = lit(1.)));
  init_scalar->options().set("physical_model", physical_model);
  init_scalar->options().set(solver::Tags::regions(), std::vector<common::URI>(1, mesh->topology().uri()));
  init_scalar->execute();

  Handle<ProtoAction> init_bnd = root.create_component<ProtoAction>("InitBnd");
  init_bnd->set_expression(nodes_expression(T = lit(0.)));
  init_bnd->options().set("physical_model", physical_model);
  init_bnd->options().set(solver::Tags::regions(), loop_regions);
  init_bnd->execute();

  Handle<ProtoAction> mat_action = root.create_component<ProtoAction>("MassMatrixAction");
  mat_action->set_expression(elements_expression(
    boost::mpl::vector1<mesh::LagrangeP1::Triag2D>(),
    group
    (
      _A(T) = _0, _a[T] = _0,
      element_quadrature
      (
        _A(T,T) += transpose(nabla(T)) * nabla(T),
        _a[T] += transpose(N(T))
      ),
      _cout << "_a before weight:" << transpose(_a) << "\n",
      apply_weight(_a, nodal_values(T)),
      _cout << "_a after weight:" << transpose(_a) << "\n",
      matrix += _A,
      sys_rhs += _a
    )
  ));

  mat_action->options().set("physical_model", physical_model);
  mat_action->options().set(solver::Tags::regions(), std::vector<common::URI>(1, mesh->topology().uri()));
  mat_action->execute();

  lss->rhs()->print_native(std::cout);

  for(Uint i = 0; i != 4; ++i)
  {
    Real result = 0;
    lss->rhs()->get_value(i, result);
    if(i == 0 || i == 2)
    {
      BOOST_CHECK_EQUAL(result, 0.);
    }
  }
}

BOOST_AUTO_TEST_CASE( CleanUp )
{
  root.remove_component("rhs_lss");
  root.remove_component("zero_lss");
  common::PE::Comm::instance().finalize();
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().is_active(),false);
}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
