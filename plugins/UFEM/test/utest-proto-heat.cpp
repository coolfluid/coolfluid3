// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/test/unit_test.hpp>

#define BOOST_PROTO_MAX_ARITY 10                        //explained in boost doc
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
 #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
 #define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10
#endif

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"

#include "mesh/LagrangeP1/Line1D.hpp"
#include "solver/Model.hpp"

#include "math/LSS/SolveLSS.hpp"
#include <math/LSS/ZeroLSS.hpp>

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "mesh/SimpleMeshGenerator.hpp"

#include "UFEM/LSSAction.hpp"
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

struct ProtoHeatFixture
{
  ProtoHeatFixture() :
    root( Core::instance().root() )
  {
  }

  Component& root;
};

BOOST_FIXTURE_TEST_SUITE( ProtoHeatSuite, ProtoHeatFixture )

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
}

BOOST_AUTO_TEST_CASE( Heat1DComponent )
{
  Core::instance().environment().options().set("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 5 ;

  // Setup a model
  Model& model = *root.create_component<Model>("Model");
  Domain& domain = model.create_domain("Domain");
  UFEM::Solver& solver = *model.create_component<UFEM::Solver>("Solver");

  Handle<UFEM::LSSAction> lss_action(solver.add_direct_solver("cf3.UFEM.LSSAction"));

  // Proto placeholders
  FieldVariable<0, ScalarField> temperature("Temperature", UFEM::Tags::solution());

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Line1D> allowed_elements;

  // BCs
  boost::shared_ptr<UFEM::BoundaryConditions> bc = allocate_component<UFEM::BoundaryConditions>("BoundaryConditions");

  // add the top-level actions (assembly, BC and solve)
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
          element_quadrature( _A(temperature) += transpose(nabla(temperature)) * nabla(temperature) ),
          lss_action->system_matrix += _A
        )
      )
    )
    << bc
    << allocate_component<math::LSS::SolveLSS>("SolveLSS")
    << create_proto_action("Increment", nodes_expression(temperature += lss_action->solution(temperature)))
    << create_proto_action("Output", nodes_expression(_cout << "T(" << coordinates(0,0) << ") = " << temperature << "\n"))
    << create_proto_action("CheckResult", nodes_expression(_check_close(temperature, 10. + 25.*(coordinates(0,0) / length), 1e-6)));

  // Setup physics
  model.create_physics("cf3.UFEM.NavierStokesPhysics");

  // Setup mesh
  // Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  // Tools::MeshGeneration::create_line(mesh, length, nb_segments);
  boost::shared_ptr<MeshGenerator> create_line = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","create_line");
  create_line->options().set("mesh",domain.uri()/"Mesh");
  create_line->options().set("lengths",std::vector<Real>(DIM_1D, length));
  create_line->options().set("nb_cells",std::vector<Uint>(DIM_1D, nb_segments));
  Mesh& mesh = create_line->generate();
  
  lss_action->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));

  // Set boundary conditions
  bc->add_constant_bc("xneg", "Temperature", 10.);
  bc->add_constant_bc("xpos", "Temperature", 35.);

  // Run the solver
  model.simulate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
