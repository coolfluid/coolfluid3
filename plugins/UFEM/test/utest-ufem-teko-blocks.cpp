// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test teko matrix blocking functions"

#include <boost/assign.hpp>
#include <boost/test/unit_test.hpp>

#include <Teko_EpetraHelpers.hpp>
#include <Teko_Utilities.hpp>

#include <Thyra_EpetraLinearOp.hpp>
#include <Thyra_VectorStdOps.hpp>

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "common/PE/CommPattern.hpp"

#include "math/LSS/System.hpp"
#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/Trilinos/TrilinosCrsMatrix.hpp"
#include "math/LSS/Trilinos/TekoBlockedOperator.hpp"
#include "math/LSS/Trilinos/TrilinosVector.hpp"

#include "mesh/Domain.hpp"
#include "mesh/LagrangeP1/Line1D.hpp"

#include "solver/Model.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "mesh/MeshGenerator.hpp"

#include "UFEM/LSSAction.hpp"
#include "UFEM/Solver.hpp"
#include "UFEM/SparsityBuilder.hpp"
#include "UFEM/Tags.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::common;
using namespace cf3::math;
using namespace cf3::math::Consts;
using namespace cf3::mesh;

using namespace boost::assign;

struct UFEMBuildSparsityFixture
{
  UFEMBuildSparsityFixture() :
    root( Core::instance().root() )
  {
  }
  
  void apply_block(const int i, const int j, Teuchos::RCP<Epetra_Vector>& output)
  {
    const Teuchos::RCP<const Epetra_Operator> block = blocked_op->GetBlock(i,j);
    Epetra_Vector testvec(block->OperatorDomainMap());
    int num_entries = block->OperatorDomainMap().NumMyElements();
    std::vector<int> indices(num_entries);
    for(int i = 0; i != num_entries; ++i)
      indices[i] = i;
    if(j ==0)
      testvec.ReplaceMyValues(num_entries, &u_test_vec[0], &indices[0]);
    else
      testvec.ReplaceMyValues(num_entries, &p_test_vec[0], &indices[0]);

    output = Teuchos::rcp(new Epetra_Vector(block->OperatorRangeMap()));
    block->Apply(testvec, *output);
  }

  Component& root;
  Teuchos::RCP<Teko::Epetra::BlockedEpetraOperator> blocked_op;
  std::vector<Real> u_test_vec, p_test_vec;
};



BOOST_FIXTURE_TEST_SUITE( UFEMBuildSparsitySuite, UFEMBuildSparsityFixture )

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
}

BOOST_AUTO_TEST_CASE( Blocked2DQuads )
{
  Core::instance().environment().options().set("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 3;
  const Uint nb_nodes = (nb_segments+1) * (nb_segments+1);

  // Setup a model
  Model& model = *root.create_component<Model>("Model");
  Domain& domain = model.create_domain("Domain");

  UFEM::Solver& solver = *model.create_component<UFEM::Solver>("Solver");

  Handle<UFEM::LSSAction> lss_action(solver.add_direct_solver("cf3.UFEM.LSSAction"));
  lss_action->options().set("blocked_system", false);

  // Proto placeholders
  FieldVariable<0, VectorField> u("u", UFEM::Tags::solution());
  FieldVariable<1, ScalarField> p("p", UFEM::Tags::solution());

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Quad2D> allowed_elements;

  // add the top-level actions (assembly, BC and solve)
  lss_action->add_component(create_proto_action
    (
      "Assembly",
      elements_expression
      (
        allowed_elements,
        group
        (
          _A = _0,
          element_quadrature
          (
            _A(u[_i], u[_i]) += transpose(N(u))*N(u),
            _A(u[_i],p) += -transpose(nabla(u)[_i])*N(p),
            _A(p,u[_i]) += transpose(N(p))*nabla(u)[_i],
            _A(p,p) += transpose(nabla(p))*nabla(p)
          ),
          lss_action->system_matrix += _A
        )
      )
    ));
  
  physics::PhysModel& physical_model = model.create_physics("cf3.UFEM.NavierStokesPhysics");

  // Setup mesh
  Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  BlockMesh::BlockArrays& blocks = *domain.create_component<BlockMesh::BlockArrays>("blocks");

  *blocks.create_points(2, 4) << 0. << 0. << length << 0. << length << length << 0. << length;
  *blocks.create_blocks(1) << 0 << 1 << 2 << 3;
  *blocks.create_block_subdivisions() << nb_segments << nb_segments;
  *blocks.create_block_gradings() << 1. << 1. << 1. << 1.;

  *blocks.create_patch("bottom", 1) << 0 << 1;
  *blocks.create_patch("right", 1) << 1 << 2;
  *blocks.create_patch("top", 1) << 2 << 3;
  *blocks.create_patch("left", 1) << 3 << 0;

  blocks.partition_blocks(PE::Comm::instance().size(), YY);
  blocks.create_mesh(mesh);

  lss_action->options().set("matrix_builder", std::string("cf3.math.LSS.TrilinosCrsMatrix"));
  LSS::System& lss = lss_action->create_lss();
  lss_action->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));
  
  model.simulate();

  // Initialize the solution vector
  u_test_vec.resize(nb_nodes*2);
  p_test_vec.resize(nb_nodes);
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    lss.solution()->set_value(i, 0, i*3);
    lss.solution()->set_value(i, 1, i*3+1);
    lss.solution()->set_value(i, 2, i*3+2);
    u_test_vec[i*2] = i*3;
    u_test_vec[i*2+1] = i*3+1;
    p_test_vec[i] = i*3+2;
  }
  
  // Apply the complete matrix to the solution vector
  Handle<math::LSS::TrilinosCrsMatrix> crs_mat(lss.matrix());
  Handle<math::LSS::TrilinosVector> tri_sol(lss.solution());
  Handle<math::LSS::TrilinosVector> tri_rhs(lss.rhs());
  BOOST_CHECK(crs_mat);
  crs_mat->epetra_matrix()->Apply(*tri_sol->epetra_vector(), *tri_rhs->epetra_vector());

  // Create matrix subblocks
  math::VariablesDescriptor& descriptor = common::find_component_with_tag<VariablesDescriptor>(physical_model.variable_manager(), UFEM::Tags::solution());
  blocked_op = math::LSS::create_teko_blocked_operator(*crs_mat, descriptor);
  
  Teuchos::RCP<Epetra_Vector> uu_output;
  apply_block(0, 0, uu_output);
  
  Teuchos::RCP<Epetra_Vector> up_output;
  apply_block(0, 1, up_output);
  
  Teuchos::RCP<Epetra_Vector> pu_output;
  apply_block(1, 0, pu_output);
  
  Teuchos::RCP<Epetra_Vector> pp_output;
  apply_block(1, 1, pp_output);
  
  // Check if the result of applying blocks is the same as the result of applying the whole matrix
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    Real u_check, v_check, p_check;
    tri_rhs->get_value(i, 0, u_check);
    tri_rhs->get_value(i, 1, v_check);
    tri_rhs->get_value(i, 2, p_check);
    
    BOOST_CHECK_CLOSE((*uu_output)[i*2] + (*up_output)[i*2], u_check, 1e-8);
    BOOST_CHECK_CLOSE((*uu_output)[i*2+1] + (*up_output)[i*2+1], v_check, 1e-8);
    BOOST_CHECK_CLOSE((*pu_output)[i] + (*pp_output)[i], p_check, 1e-8);
  }

  Teuchos::RCP<const Thyra::LinearOpBase<Real> > Aup = Thyra::epetraLinearOp(blocked_op->GetBlock(0,1));
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > Apu = Thyra::epetraLinearOp(blocked_op->GetBlock(1,0));
  Teuchos::RCP<const Thyra::LinearOpBase<Real> > K = Teko::explicitMultiply(Apu,Aup);
  Teuchos::RCP<Thyra::VectorBase<Real> > b = Thyra::createMember(K->range());
  Teuchos::RCP<Thyra::VectorBase<Real> > x = Thyra::createMember(K->domain());

  for(Uint i = 0; i != nb_nodes; ++i)
  {
    Thyra::set_ele(i, p_test_vec[i], b.ptr());
  }
  Thyra::apply(*K, Thyra::NOTRANS, *b, x.ptr());

  for(Uint i = 0; i != nb_nodes; ++i)
  {
    BOOST_CHECK((*pp_output)[i] != Thyra::get_ele(*x, i));
  }

  Teuchos::RCP<std::ofstream> k_out = Teuchos::rcp(new  std::ofstream("K.txt", std::ios::out));
  Teuchos::RCP<Teuchos::FancyOStream> k_fancy_out = Teuchos::fancyOStream(k_out);
  Thyra::describeLinearOp(*K, *k_fancy_out, Teuchos::VERB_EXTREME);

  Teuchos::RCP<const Thyra::LinearOpBase<Real> > App = Thyra::epetraLinearOp(blocked_op->GetBlock(1,1));
  Teuchos::RCP<std::ofstream> app_out = Teuchos::rcp(new  std::ofstream("App.txt", std::ios::out));
  Teuchos::RCP<Teuchos::FancyOStream> app_fancy_out = Teuchos::fancyOStream(app_out);
  Thyra::describeLinearOp(*App, *app_fancy_out, Teuchos::VERB_EXTREME);
}

BOOST_AUTO_TEST_CASE( FinalizeMPI )
{
  common::PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
