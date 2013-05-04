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
#include <Thyra_MultiVectorStdOps.hpp>
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
      
      common::Core::instance().environment().options().set("log_level", 4);
      model = Core::instance().root().create_component<Model>("Model");
      physical_model = Handle<physics::PhysModel>(model->create_physics("cf3.physics.DynamicModel").handle());
      Domain& dom = model->create_domain("Domain");
      mesh = dom.create_component<Mesh>("mesh");
      Tools::MeshGeneration::create_rectangle(*mesh, 5., 5., 5, 5);
      
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
          BOOST_FOREACH(const Uint node_a, connectivity[elem])
          {
            BOOST_FOREACH(const Uint node_b, connectivity[elem])
            {
              connectivity_sets[node_a].insert(node_b);
            }
          }
        }
      }
      
      starting_indices.push_back(0);
      BOOST_FOREACH(const std::set<Uint>& nodes, connectivity_sets)
      {
        starting_indices.push_back(starting_indices.back() + nodes.size());
        node_connectivity.insert(node_connectivity.end(), nodes.begin(), nodes.end());
      }
      
      loop_regions.push_back(mesh->topology().uri());
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

struct CustomTerminal
{
  /// Custom ops must implement the  TR1 result_of protocol
  template<typename Signature>
  struct result;

  template<typename This, typename DataT>
  struct result<This(DataT)>
  {
    typedef const Eigen::Matrix<Real, DataT::SupportShapeFunction::nb_nodes, DataT::SupportShapeFunction::nb_nodes>& type;
  };

  template<typename StorageT, typename DataT>
  const StorageT& operator()(StorageT& result, const DataT& field) const
  {
    result.setConstant(m_constant);
    return result;
  }

  // Set the constant to use for setting the matrix
  void set_constant(const Real c)
  {
    m_constant = c;
  }

  Real m_constant;
};

BOOST_AUTO_TEST_CASE( MyTerminal )
{
  SFOp< CustomSFOp<CustomTerminal> > my_term;

  // Create an action that can wrap an expression
  Handle<ProtoAction> action = root.create_component<ProtoAction>("MyTermAction");
  action->set_expression(elements_expression(_cout << lit( my_term ) << "\n"));
  action->options().set("physical_model", physical_model);
  action->options().set(solver::Tags::regions(), loop_regions);

  my_term.op.set_constant(2.);
  
  // Run the action
  action->execute();
}

struct ScalarLSSVector
{
  /// Custom ops must implement the  TR1 result_of protocol
  template<typename Signature>
  struct result;

  template<typename This, typename DataT>
  struct result<This(DataT)>
  {
    typedef const Eigen::Matrix<Real, DataT::SupportShapeFunction::nb_nodes, 1>& type;
  };

  template<typename StorageT, typename DataT>
  const StorageT& operator()(StorageT& result, const DataT& data) const
  {
    cf3_assert(result.rows() == data.block_accumulator.sol.rows());
    m_vector->get_sol_values(data.block_accumulator);
    result = data.block_accumulator.sol;
    return result;
  }

  // Set the constant to use for setting the matrix
  void set_vector(const Handle<math::LSS::Vector>& v)
  {
    m_vector = v;
  }

  Handle<math::LSS::Vector> m_vector;
};

struct VectorLSSVector
{
  /// Custom ops must implement the  TR1 result_of protocol
  template<typename Signature>
  struct result;

  template<typename This, typename DataT>
  struct result<This(DataT)>
  {
    typedef const Eigen::Matrix<Real, DataT::dimension*DataT::SupportShapeFunction::nb_nodes, 1>& type;
  };

  template<typename StorageT, typename DataT>
  const StorageT& operator()(StorageT& result, const DataT& data) const
  {
    cf3_assert(result.rows() == data.block_accumulator.sol.rows());
    m_vector->get_sol_values(data.block_accumulator);
    // We need to renumber to the blocked structure used in the element matrices
    for(Uint j = 0; j != DataT::dimension; ++j)
    {
      const Uint offset = j*DataT::SupportShapeFunction::nb_nodes;
      for(Uint i = 0; i != DataT::SupportShapeFunction::nb_nodes; ++i)
      {
        result[offset + i] = data.block_accumulator.sol[i*DataT::dimension + j];
      }
    }
    return result;
  }

  // Set the constant to use for setting the matrix
  void set_vector(const Handle<math::LSS::Vector>& v)
  {
    m_vector = v;
  }

  Handle<math::LSS::Vector> m_vector;
};

struct CustomLaplacianApply
{
  // Custom ops must implement the  TR1 result_of protocol
  template<typename Signature>
  struct result;

  template<typename This, typename TempT, typename MatrixT, typename VectorT>
  struct result<This(TempT, MatrixT, VectorT)>
  {
    typedef const VectorT& type;
  };

  template<typename StorageT, typename TempT, typename MatrixT, typename VectorT>
  const StorageT& operator()(StorageT& result, TempT& T, const MatrixT& m, const VectorT& vec) const
  {
    result = m*vec;
    return result;
  }
};

MakeSFOp<CustomLaplacianApply>::type laplacian_apply = {};

BOOST_AUTO_TEST_CASE( ScalarTest )
{
  Handle<math::LSS::System> lss = root.create_component<math::LSS::System>("scalar_lss");
  lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  lss->create(mesh->geometry_fields().comm_pattern(), 1, node_connectivity, starting_indices);
  
  Handle<math::LSS::ThyraOperator> op(lss->matrix());
  Handle<math::LSS::ThyraVector> solution(lss->solution());
  Handle<math::LSS::ThyraVector> rhs(lss->rhs());
  
  // Set random solution
  Thyra::randomize(0., 1., solution->thyra_vector().ptr());
  
  Handle<ProtoAction> action = root.create_component<ProtoAction>("ScalarLSSAction");
  
  // Terminals to use
  FieldVariable<0, ScalarField> T("ScalarVar", "scalar");
  SystemMatrix matrix(*lss);
  SystemRHS sys_rhs(*lss);
  SolutionVector sol_vec(*lss);

  Handle<math::LSS::Vector> vec_copy(root.create_component("ScalarVector", "cf3.math.LSS.TrilinosVector"));
  lss->solution()->clone_to(*vec_copy);
  SFOp< CustomSFOp<ScalarLSSVector> > scalar_vector;
  scalar_vector.op.set_vector(vec_copy);
  
  // Run the expression
  action->set_expression(elements_expression(
    group
    (
      _A = _0,
      element_quadrature
      (
        _A(T,T) += transpose(nabla(T)) * nabla(T)
      ),      
      matrix += _A,
      sys_rhs += _A * scalar_vector
    )
  ));
  
  action->options().set("physical_model", physical_model);
  action->options().set(solver::Tags::regions(), loop_regions);
  
  field_manager->create_field("scalar", mesh->geometry_fields());
  
  // Set the field to random
  for_each_node(mesh->topology(), T = sol_vec(T));
  
  action->execute();
  
  
  Teuchos::RCP< Thyra::MultiVectorBase<Real> > rhs2 = Thyra::createMembers(op->thyra_operator()->range(), 1);
  Thyra::apply(*op->thyra_operator(), Thyra::NOTRANS, *solution->thyra_vector(), rhs2.ptr());
  
  std::vector<Real> diff_norm(1);
  
  Thyra::norms(*rhs2, Teuchos::arrayViewFromVector(diff_norm));
  std::cout << "rhs2 norm: " << diff_norm.front() << std::endl;
  BOOST_CHECK(diff_norm.front() > 1e-6);
  
  Thyra::update(-1., *rhs->thyra_vector(), rhs2.ptr());
  Thyra::norms(*rhs2, Teuchos::arrayViewFromVector(diff_norm));
  std::cout << "diff norm: " << diff_norm.front() << std::endl;
  BOOST_CHECK_SMALL(diff_norm.front(), 1e-10);
}

BOOST_AUTO_TEST_CASE( VectorTest )
{
  Handle<math::LSS::System> lss = root.create_component<math::LSS::System>("vector_lss");
  lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  
  Handle<ProtoAction> action = root.create_component<ProtoAction>("VectorLSSAction");
  // Create this so we have an option for the LSS
  Handle<math::LSS::SolveLSS> solve_action = root.create_component<math::LSS::SolveLSS>("SolveLSS");
  
  // Terminals to use
  FieldVariable<0, VectorField> T("VectorVar", "vector");
  SystemMatrix matrix(solve_action->options().option("lss"));
  SystemRHS sys_rhs(solve_action->options().option("lss"));
  SolutionVector sol_vec(solve_action->options().option("lss"));
  SFOp< CustomSFOp<VectorLSSVector> > vector_vector;
  
  // Run the expression
  action->set_expression(elements_expression(
    group
    (
      _A = _0,
      element_quadrature
      (
        _A(T[_i],T[_i]) += transpose(nabla(T)) * nabla(T)
      ),      
      matrix += _A,
      sys_rhs += _A * vector_vector
    )
  ));
  
  action->options().set("physical_model", physical_model);
  action->options().set(solver::Tags::regions(), loop_regions);
  
  field_manager->create_field("vector", mesh->geometry_fields());
  
  //lss->create(mesh->geometry_fields().comm_pattern(), 2, node_connectivity, starting_indices);
  lss->create_blocked(mesh->geometry_fields().comm_pattern(), *Handle<math::VariablesDescriptor>(physical_model->variable_manager().get_child("vector")), node_connectivity, starting_indices);
  solve_action->options().set("lss", lss);
  
  Handle<math::LSS::ThyraOperator> op(lss->matrix());
  Handle<math::LSS::ThyraVector> solution(lss->solution());
  Handle<math::LSS::ThyraVector> rhs(lss->rhs());
  
  // Set random solution
  Thyra::randomize(0., 1., solution->thyra_vector().ptr());
  
  Handle<math::LSS::Vector> vec_copy(root.create_component("VectorVector", "cf3.math.LSS.TrilinosVector"));
  lss->solution()->clone_to(*vec_copy);
  vector_vector.op.set_vector(vec_copy);
  
  // Set the field to random
  for_each_node(mesh->topology(), T = sol_vec(T));
  
  action->execute();
  
  Teuchos::RCP< Thyra::MultiVectorBase<Real> > rhs2 = Thyra::createMembers(op->thyra_operator()->range(), 1);
  Thyra::apply(*op->thyra_operator(), Thyra::NOTRANS, *solution->thyra_vector(), rhs2.ptr());
  
  std::vector<Real> diff_norm(1);
  
  Thyra::norms(*rhs2, Teuchos::arrayViewFromVector(diff_norm));
  std::cout << "rhs2 norm: " << diff_norm.front() << std::endl;
  BOOST_CHECK(diff_norm.front() > 1e-6);
  
  Thyra::update(-1., *rhs->thyra_vector(), rhs2.ptr());
  Thyra::norms(*rhs2, Teuchos::arrayViewFromVector(diff_norm));
  std::cout << "diff norm: " << diff_norm.front() << std::endl;
  BOOST_CHECK_SMALL(diff_norm.front(), 1e-10);
}

BOOST_AUTO_TEST_CASE( NestedCustomOps )
{
  Handle<math::LSS::System> lss = root.create_component<math::LSS::System>("nesting_lss");
  lss->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  lss->create(mesh->geometry_fields().comm_pattern(), 1, node_connectivity, starting_indices);

  Handle<math::LSS::ThyraOperator> op(lss->matrix());
  Handle<math::LSS::ThyraVector> solution(lss->solution());
  Handle<math::LSS::ThyraVector> rhs(lss->rhs());

  // Set random solution
  Thyra::randomize(0., 1., solution->thyra_vector().ptr());

  Handle<ProtoAction> action = root.create_component<ProtoAction>("ScalarLSSAction");

  // Terminals to use
  FieldVariable<0, ScalarField> T("ScalarVar2", "scalar2");
  SystemMatrix matrix(*lss);
  SystemRHS sys_rhs(*lss);
  SolutionVector sol_vec(*lss);

  Handle<math::LSS::Vector> vec_copy(root.create_component("ScalarVector2", "cf3.math.LSS.TrilinosVector"));
  lss->solution()->clone_to(*vec_copy);
  SFOp< CustomSFOp<ScalarLSSVector> > scalar_vector;
  scalar_vector.op.set_vector(vec_copy);

  boost::mpl::vector1<mesh::LagrangeP1::Quad2D> etype;

  // Run the expression
  action->set_expression(elements_expression(etype,
    group
    (
      _A = _0,
      element_quadrature
      (
        _A(T,T) += transpose(nabla(T)) * nabla(T)
      ),
      matrix += _A,
      sys_rhs += laplacian_apply(T, _A, lit(scalar_vector))
    )
  ));

  action->options().set("physical_model", physical_model);
  action->options().set(solver::Tags::regions(), loop_regions);

  field_manager->create_field("scalar2", mesh->geometry_fields());

  // Set the field to random
  for_each_node(mesh->topology(), T = sol_vec(T));

  action->execute();


  Teuchos::RCP< Thyra::MultiVectorBase<Real> > rhs2 = Thyra::createMembers(op->thyra_operator()->range(), 1);
  Thyra::apply(*op->thyra_operator(), Thyra::NOTRANS, *solution->thyra_vector(), rhs2.ptr());

  std::vector<Real> diff_norm(1);

  Thyra::norms(*rhs2, Teuchos::arrayViewFromVector(diff_norm));
  std::cout << "rhs2 norm: " << diff_norm.front() << std::endl;
  BOOST_CHECK(diff_norm.front() > 1e-6);

  Thyra::update(-1., *rhs->thyra_vector(), rhs2.ptr());
  Thyra::norms(*rhs2, Teuchos::arrayViewFromVector(diff_norm));
  std::cout << "diff norm: " << diff_norm.front() << std::endl;
  BOOST_CHECK_SMALL(diff_norm.front(), 1e-10);
}

BOOST_AUTO_TEST_CASE( CleanUp )
{
  root.remove_component("scalar_lss");
  root.remove_component("vector_lss");
}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
