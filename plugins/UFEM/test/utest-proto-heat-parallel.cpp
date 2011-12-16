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
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "common/PE/debug.hpp"

#include "math/LSS/System.hpp"

#include "mesh/Domain.hpp"

#include "mesh/LagrangeP1/Line1D.hpp"
#include "solver/CModel.hpp"

#include "solver/actions/SolveLSS.hpp"
#include "solver/actions/Proto/CProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/LinearSolver.hpp"
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
    solver_config = boost::unit_test::framework::master_test_suite().argv[1];
  }

  Component& root;
  std::string solver_config;

};

BOOST_FIXTURE_TEST_SUITE( ProtoHeatSuite, ProtoHeatFixture )

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  //PE::wait_for_debugger(0);
}

BOOST_AUTO_TEST_CASE( Heat2DParallel)
{
  Core::instance().environment().options().configure_option("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 16 ;

  // Setup a model
  CModel& model = *root.create_component<CModel>("Model");
  Domain& domain = model.create_domain("Domain");
  UFEM::LinearSolver& solver = *model.create_component<UFEM::LinearSolver>("Solver");

  math::LSS::System& lss = *model.create_component<math::LSS::System>("LSS");
  lss.options().configure_option("solver", std::string("Trilinos"));
  solver.options().configure_option("lss", lss.handle<math::LSS::System>());

  // Proto placeholders
  MeshTerm<0, ScalarField> temperature("Temperature", UFEM::Tags::solution());

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Quad2D> allowed_elements;

  // BCs
  boost::shared_ptr<UFEM::BoundaryConditions> bc = allocate_component<UFEM::BoundaryConditions>("BoundaryConditions");

  // add the top-level actions (assembly, BC and solve)
  solver
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
          solver.system_matrix += _A
        )
      )
    )
    << bc
    << allocate_component<solver::actions::SolveLSS>("SolveLSS")
    << create_proto_action("Increment", nodes_expression(temperature += solver.solution(temperature)))
    << create_proto_action("CheckResult", nodes_expression(_check_close(temperature, 10. + 25.*(coordinates(0,0) / length), 1e-6)));

  // Setup physics
  model.create_physics("cf3.physics.DynamicModel");

  // Setup mesh
  Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  BlockMesh::BlockData& blocks = *domain.create_component<BlockMesh::BlockData>("blocks");
  blocks.dimension = 2;
  blocks.scaling_factor = 1.;
  blocks.points += list_of(0.)(0.), list_of(length)(0.), list_of(length)(length), list_of(0.)(length);
  blocks.block_points += list_of(0)(1)(2)(3);
  blocks.block_subdivisions += list_of(nb_segments)(nb_segments);
  blocks.block_gradings += list_of(1.)(1.)(1.)(1.);
  blocks.patch_names += "bottom", "right", "top",  "left";
  blocks.patch_types += "wall", "wall",  "wall", "wall";
  blocks.patch_points += list_of(0)(1), list_of(1)(2), list_of(2)(3), list_of(3)(0);
  blocks.block_distribution += 0, 1;

  BlockMesh::BlockData& parallel_blocks = *domain.create_component<BlockMesh::BlockData>("parallel_blocks");
  Mesh& serial_block_mesh = *model.create_component<Mesh>("serial_block_mesh");
  BlockMesh::partition_blocks(blocks, PE::Comm::instance().size(), XX, parallel_blocks);
  BlockMesh::build_mesh(parallel_blocks, mesh, 1);

  lss.matrix()->options().configure_option("settings_file", std::string(boost::unit_test::framework::master_test_suite().argv[1]));

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
