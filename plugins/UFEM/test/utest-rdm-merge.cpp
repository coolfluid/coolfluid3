
// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/test/unit_test.hpp>

#define BOOST_PROTO_MAX_ARITY 10
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
  #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
#endif
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"

#include "mesh/LagrangeP1/Triag2D.hpp"
#include "solver/ModelUnsteady.hpp"
#include "solver/Time.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/LSSActionUnsteady.hpp"
#include "UFEM/Solver.hpp"
#include "UFEM/Tags.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::common;
using namespace cf3::math::Consts;
using namespace cf3::mesh;

using namespace boost;


typedef std::vector<std::string> StringsT;
typedef std::vector<Uint> SizesT;

/// Check close, for testing purposes
inline void
check_close(const Real a, const Real b, const Real threshold)
{
  BOOST_CHECK_CLOSE(a, b, threshold);
}

static boost::proto::terminal< void(*)(Real, Real, Real) >::type const _check_close = {&check_close};

struct RDMMergeFixture
{
  RDMMergeFixture() :
    root( Core::instance().root() )
  {
  }

  Component& root;

};

BOOST_FIXTURE_TEST_SUITE( RDMMergeSuite, RDMMergeFixture )

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
}

BOOST_AUTO_TEST_CASE( Heat1DComponent )
{
  Core::instance().environment().options().set("log_level", 4u);

  // Parameters
  Real length            = 1.;
  const Uint nb_segments = 25 ;

  // Setup a model
  ModelUnsteady& model = *root.create_component<ModelUnsteady>("Model");
  Domain& domain = model.create_domain("Domain");
  UFEM::Solver& solver = *model.create_component<UFEM::Solver>("Solver");
  Handle<UFEM::LSSActionUnsteady> lss_action(solver.add_unsteady_solver("cf3.UFEM.LSSActionUnsteady"));
  Handle<common::ActionDirector> ic(solver.get_child("InitialConditions"));


  
  boost::shared_ptr<solver::actions::Iterate> time_loop = allocate_component<solver::actions::Iterate>("TimeLoop");
  time_loop->create_component<solver::actions::CriterionTime>("CriterionTime");

  // Proto placeholders
  FieldVariable<0, ScalarField> fi("FI", UFEM::Tags::solution());

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Triag2D> allowed_elements;

  // BCs
  boost::shared_ptr<UFEM::BoundaryConditions> bc = allocate_component<UFEM::BoundaryConditions>("BoundaryConditions");

  FieldVariable<1, VectorField> u_adv("AdvectionSpeed", "linearized_velocity");
  RealVector u_ref(2); u_ref << 1.,0.;

  // add the top-level actions (assembly, BC and solve)
  *ic
    << create_proto_action
    (
      "SetInitial",
      nodes_expression
      (
        fi = 5.
      )
    )
    << create_proto_action
    (
      "InitAdvectionSpeed",
      nodes_expression
      (
        u_adv = coordinates[1] * u_ref
      )
    );
    *lss_action
      << create_proto_action
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
              _A(fi,fi) += 0.0025 * transpose(nabla(fi)) * nabla(fi),
              _A(fi,fi) += transpose(N(fi) /*+ 0.*/ ) * u_adv*nabla(fi)
            ),
            lss_action->system_matrix += _A,
            lss_action->system_rhs += -_A * _x
          )
        )
      )
      << bc
      << allocate_component<math::LSS::SolveLSS>("SolveLSS")
      << create_proto_action("Increment", nodes_expression(fi += lss_action->solution(fi)));

  // Setup physics
  model.create_physics("cf3.physics.DynamicModel");

  // Setup mesh
  Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  Tools::MeshGeneration::create_rectangle_tris(mesh, length, length, nb_segments, nb_segments);

  lss_action->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));
  ic->get_child("SetInitial")->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));
  ic->get_child("InitAdvectionSpeed")->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));

  // Set boundary conditions
  bc->add_constant_bc("top",    "FI", 8.);
  bc->add_constant_bc("bottom", "FI", 2.);
  bc->add_constant_bc("left",   "FI", 8.);
  bc->add_constant_bc("right",  "FI", 2.);

  model.create_time();
  model.time().options().set("end_time", 10.);
  model.time().options().set("time_step",1.);

  // Run the solver
  model.simulate();

  domain.write_mesh(URI("rdmmerge.plt"));
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
