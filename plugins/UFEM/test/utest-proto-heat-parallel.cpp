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

#include "common/Core.hpp"
#include "common/CEnv.hpp"
#include "common/CRoot.hpp"

#include "common/PE/debug.hpp"

#include "Math/LSS/System.hpp"

#include "mesh/CDomain.hpp"

#include "mesh/LagrangeP1/Line1D.hpp"
#include "Solver/CModel.hpp"

#include "Solver/Actions/Proto/CProtoAction.hpp"
#include "Solver/Actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/LinearSolver.hpp"
#include "UFEM/Tags.hpp"

using namespace cf3;
using namespace cf3::Solver;
using namespace cf3::Solver::Actions;
using namespace cf3::Solver::Actions::Proto;
using namespace cf3::common;
using namespace cf3::Math::Consts;
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

  CRoot& root;
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
  Core::instance().environment().configure_option("log_level", 4u);

  // Parameters
  Real length            = 5.;
  const Uint nb_segments = 16 ;

  // Setup a model
  CModel& model = root.create_component<CModel>("Model");
  CDomain& domain = model.create_domain("Domain");
  UFEM::LinearSolver& solver = model.create_component<UFEM::LinearSolver>("Solver");

  Math::LSS::System& lss = model.create_component<Math::LSS::System>("LSS");
  lss.configure_option("solver", std::string("Trilinos"));
  solver.configure_option("lss", lss.uri());

  // Proto placeholders
  MeshTerm<0, ScalarField> temperature("Temperature", UFEM::Tags::solution());

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Quad2D> allowed_elements;

  // add the top-level actions (assembly, BC and solve)
  solver
    << create_proto_action
    (
      "Assembly",
      elements_expression
      (
        allowed_elements,
        group <<
        (
          _A = _0,
          element_quadrature( _A(temperature) += transpose(nabla(temperature)) * nabla(temperature) ),
          solver.system_matrix += _A
        )
      )
    )
    << solver.boundary_conditions()
    << solver.solve_action()
    << create_proto_action("Increment", nodes_expression(temperature += solver.solution(temperature)))
    << create_proto_action("CheckResult", nodes_expression(_check_close(temperature, 10. + 25.*(coordinates(0,0) / length), 1e-6)));

  // Setup physics
  model.create_physics("CF.Physics.DynamicModel");

  // Setup mesh
  CMesh& mesh = domain.create_component<CMesh>("Mesh");
  BlockMesh::BlockData& blocks = domain.create_component<BlockMesh::BlockData>("blocks");
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

  BlockMesh::BlockData& parallel_blocks = domain.create_component<BlockMesh::BlockData>("parallel_blocks");
  CMesh& serial_block_mesh = model.create_component<CMesh>("serial_block_mesh");
  BlockMesh::partition_blocks(blocks, PE::Comm::instance().size(), XX, parallel_blocks);
  BlockMesh::build_mesh(parallel_blocks, mesh, 1);

  lss.matrix()->configure_option("settings_file", std::string(boost::unit_test::framework::master_test_suite().argv[1]));

  // Set boundary conditions
  solver.boundary_conditions().add_constant_bc("left", "Temperature", 10.);
  solver.boundary_conditions().add_constant_bc("right", "Temperature", 35.);

  // Run the solver
  model.simulate();
  
  // Write data pertaining to communication
  Field& comm_field = mesh.geometry().create_field("comm", "ghosts, updatable, rank");
  const Uint nb_nodes = mesh.geometry().size();
  for(Uint node = 0; node != nb_nodes; ++node)
  {
    comm_field[node][0] = mesh.geometry().is_ghost(node) ? 1. : 0.;
    comm_field[node][1] = mesh.geometry().comm_pattern().isUpdatable()[node];
    comm_field[node][2] = mesh.geometry().rank()[node];
  }

  // Save
  model.domain().create_component("writer", "CF.Mesh.VTKXML.CWriter");
  model.domain().write_mesh(URI("utest-proto-heat-parallel_output.pvtu", cf3::common::URI::Scheme::FILE));
//   lss.matrix()->print("utest-proto-heat-parallel_matrix-" + boost::lexical_cast<std::string>(common::PE::Comm::instance().rank()) + ".plt");
//   lss.rhs()->print("utest-proto-heat-parallel_rhs-" + boost::lexical_cast<std::string>(common::PE::Comm::instance().rank()) + ".plt");
//   lss.solution()->print("utest-proto-heat-parallel_solution-" + boost::lexical_cast<std::string>(common::PE::Comm::instance().rank()) + ".plt");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
