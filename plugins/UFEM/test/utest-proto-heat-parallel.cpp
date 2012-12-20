// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

#define BOOST_PROTO_MAX_ARITY 10
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
 #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
 #define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10
#endif

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "common/PE/debug.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"

#include "mesh/LagrangeP1/Line1D.hpp"
#include "solver/Model.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

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

using namespace boost::assign;


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
  //PE::wait_for_debugger(0);
}

BOOST_AUTO_TEST_CASE( Heat2DParallel)
{
  Core::instance().environment().options().set("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 16 ;

  // Setup a model
  Model& model = *root.create_component<Model>("Model");
  Domain& domain = model.create_domain("Domain");
  UFEM::Solver& solver = *model.create_component<UFEM::Solver>("Solver");

  Handle<UFEM::LSSAction> lss_action(solver.add_direct_solver("cf3.UFEM.LSSAction"));

  // Proto placeholders
  FieldVariable<0, ScalarField> temperature("Temperature", UFEM::Tags::solution());

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Quad2D> allowed_elements;

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
    << create_proto_action("CheckResult", nodes_expression(_check_close(temperature, 10. + 25.*(coordinates(0,0) / length), 1e-6)));

  // Setup physics
  model.create_physics("cf3.physics.DynamicModel");

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

  lss_action->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));

  // Set boundary conditions
  bc->add_constant_bc("left", "Temperature", 10.);
  bc->add_constant_bc("right", "Temperature", 35.);

  // Run the solver
  model.simulate();

  // Write data pertaining to communication
  Field& comm_field = mesh.geometry_fields().create_field("comm", "ghosts, updatable, rank");
  const Uint nb_nodes = mesh.geometry_fields().size();
  for(Uint node = 0; node != nb_nodes; ++node)
  {
    comm_field[node][0] = mesh.geometry_fields().is_ghost(node) ? 1. : 0.;
    comm_field[node][1] = mesh.geometry_fields().comm_pattern().isUpdatable()[node];
    comm_field[node][2] = mesh.geometry_fields().rank()[node];
  }

  // Save
  model.domain().create_component("writer", "cf3.mesh.VTKXML.Writer");
  model.domain().write_mesh(URI("utest-proto-heat-parallel_output.pvtu", cf3::common::URI::Scheme::FILE));
//   lss.matrix()->print("utest-proto-heat-parallel_matrix-" + boost::lexical_cast<std::string>(common::PE::Comm::instance().rank()) + ".plt");
//   lss.rhs()->print("utest-proto-heat-parallel_rhs-" + boost::lexical_cast<std::string>(common::PE::Comm::instance().rank()) + ".plt");
//   lss.solution()->print("utest-proto-heat-parallel_solution-" + boost::lexical_cast<std::string>(common::PE::Comm::instance().rank()) + ".plt");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
